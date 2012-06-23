#include "KinectControl.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/*
	KinectControl error codes
 
Code	String
 0		no error.
 1		KinectControl Already Started.
 2		freenect_init() failed.
*/

KinectControl *kinectControlRef = NULL;
uint16_t t_gamma[2048];
//double t_gamma[2048];
int got_rgb = 0;
int got_depth = 0;
volatile int die = 0;
pthread_t freenect_thread;	
int color=0;


pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;

uint8_t *depth_mid, *depth_front;
uint8_t *rgb_back, *rgb_mid, *rgb_front;


freenect_context *f_ctx;
freenect_device *f_dev;
freenect_video_format requested_format;
freenect_video_format current_format;


int freenect_angle;
int freenect_led;
unsigned int user_device_number;



///////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
#pragma mark Other Funcs


void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{	
	if (depth_mid == NULL) return;
	int i;
	uint16_t *depth = (uint16_t*)v_depth;
	//depth_mid[0] = 255;
	
	pthread_mutex_lock(&gl_backbuf_mutex);
	for (i=0; i<640*480; i++) {
		int pval = t_gamma[depth[i]];		
		int lb = pval & 0xff;
		switch (pval>>8) {
			case 0:
				depth_mid[3*i+0] = 255;
				depth_mid[3*i+1] = 255-lb;
				depth_mid[3*i+2] = 255-lb;
				break;
			case 1:
				depth_mid[3*i+0] = 255;
				depth_mid[3*i+1] = lb;
				depth_mid[3*i+2] = 0;
				break;
			case 2:
				depth_mid[3*i+0] = 255-lb;
				depth_mid[3*i+1] = 255;
				depth_mid[3*i+2] = 0;
				break;
			case 3:
				depth_mid[3*i+0] = 0;
				depth_mid[3*i+1] = 255;
				depth_mid[3*i+2] = lb;
				break;
			case 4:
				depth_mid[3*i+0] = 0;
				depth_mid[3*i+1] = 255-lb;
				depth_mid[3*i+2] = 255;
				break;
			case 5:
				depth_mid[3*i+0] = 0;
				depth_mid[3*i+1] = 0;
				depth_mid[3*i+2] = 255-lb;
				break;
			default:
				depth_mid[3*i+0] = 0;
				depth_mid[3*i+1] = 0;
				depth_mid[3*i+2] = 0;
				break;
		}
	}
	 
	got_depth++;
	
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
	 
}

void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{	
	pthread_mutex_lock(&gl_backbuf_mutex);

	// swap buffers
	assert (rgb_back == rgb);
	rgb_back = rgb_mid;
	freenect_set_video_buffer(dev, rgb_back);
	rgb_mid = (uint8_t*)rgb;
	got_rgb++;
	
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}




// the freenect thread. 
void *freenect_threadfunc(void *arg) {
	
	int accelCount = 0;
	
	freenect_set_tilt_degs(f_dev,freenect_angle);
	freenect_set_led(f_dev,LED_RED);
	freenect_set_depth_callback(f_dev, depth_cb);
	freenect_set_video_callback(f_dev, rgb_cb);
	freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, current_format));
	freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	freenect_set_video_buffer(f_dev, rgb_back);
	
	freenect_start_depth(f_dev);
	freenect_start_video(f_dev);
	
	while (!die && freenect_process_events(f_ctx) >= 0) {
		if (accelCount++ >= 2000)
		{
			accelCount = 0;
			freenect_raw_tilt_state* state;
			freenect_update_tilt_state(f_dev);
			state = freenect_get_tilt_state(f_dev);
			double dx,dy,dz;
			freenect_get_mks_accel(state, &dx, &dy, &dz);
		}
		
		if (requested_format != current_format) {
			freenect_stop_video(f_dev);
			freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, requested_format));
			freenect_start_video(f_dev);
			current_format = requested_format;
		}
	}
	
	freenect_stop_depth(f_dev);
	freenect_stop_video(f_dev);
	
	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	return NULL;
}






KinectControl::KinectControl() {
	printf("\nKinectControl Init\n");
	errorCode = 0;
	isInited = false;
	if (kinectControlRef != NULL) {
		errorCode = 1;
		errorString = "KinectControl Already Started.";
		printf("%s.\n",errorString.c_str());
		return;
	}
	
	for (int i=0; i<2048; i++) {
		float v = i/2048.0;
		v = powf(v, 3)* 6;
		t_gamma[i] = v*6*256;
	}	
	
	kinectControlRef = this;
	freenect_angle = 0;
	user_device_number = -1;
	requested_format = FREENECT_VIDEO_RGB;
	current_format = FREENECT_VIDEO_RGB;
	//depth_mid = (uint8_t*)malloc(640*480*3);
	depth_front = (uint8_t*)malloc(640*480*3);
	rgb_back = (uint8_t*)malloc(640*480*3);
	rgb_mid = (uint8_t*)malloc(640*480*3);
	rgb_front = (uint8_t*)malloc(640*480*3);
	
	depth_mid = NULL;
	
	memset(depth_front,0,(640*480*3));
	memset(rgb_back,0,(640*480*3));
	memset(rgb_mid,0,(640*480*3));
	memset(rgb_front,0,(640*480*3));
	
	if (freenect_init(&f_ctx, NULL) < 0) {
		errorCode = 2;
		errorString = "freenect_init() failed.";
		printf("%s\n",errorString.c_str());
		return;
	} else {
		printf("freenect_init() OK\n");
	}

	
	freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);
	freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));
	
	int nr_devices = freenect_num_devices (f_ctx);
	printf ("Number of devices found: %d\n", nr_devices);	
	
	// check if there were no device found.
	if (nr_devices < 0) {
		freenect_shutdown(f_ctx);
		errorCode = 3;
		errorString = "No freenect devices found!";
		printf("%s\n", errorString.c_str());
	}
}



KinectControl::~KinectControl() {
	printf("Destroying KinectControl.\n");
	
	die = 1;
	
	pthread_join(freenect_thread, NULL);
	freenect_shutdown(f_ctx);
	free(depth_mid);
	free(depth_front);
	free(rgb_back);
	free(rgb_mid);
	free(rgb_front);

	kinectControlRef = NULL;
}


int KinectControl::initDevice(unsigned int _user_device_number) {
	printf("KinectControl. initing the device.\n");	
	
	user_device_number = _user_device_number;
	if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
		this->errorCode = 4;
		errorString = "Could not open device";

		printf("%s\n", errorString.c_str());
		freenect_shutdown(f_ctx);
		return this->errorCode;
	} else {
		printf("Kinect device opened.\n");
		isInited = true;

	}

	
	int res;
	res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
	if (res) {
		errorString = "pthread_create failed";
		printf("%s\n",errorString.c_str());
		freenect_shutdown(f_ctx);
		this->errorCode = 5;
		return this->errorCode;
	} else {
		printf("Kinect thread created.\n");
	}

	
	
	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Other Funcs
#pragma mark Other Funcs

void KinectControl::setDepthMid(char *_depth_mid) {
	depth_mid = (uint8_t *)_depth_mid;
}




uint8_t *KinectControl::getDepthMid() {
	uint8_t *res = depth_mid;
	return res;
}

uint8_t *KinectControl::getRGB() {
	uint8_t *res = rgb_mid;
	return res;
}


bool KinectControl::updateData() {
	
	
	pthread_mutex_lock(&gl_backbuf_mutex);

	if (current_format == FREENECT_VIDEO_YUV_RGB) {
		while (!got_depth && !got_rgb) {
			pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
		}
	} else {
		while ((!got_depth || !got_rgb) && requested_format != current_format) {
			pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
		}
	}
	
	if (requested_format != current_format) {
		pthread_mutex_unlock(&gl_backbuf_mutex);
		return false;
	}
	
	
	uint8_t *tmp;

	
	if (got_depth) {
		tmp = depth_front;
		depth_front = depth_mid;
		depth_mid = tmp;
		got_depth = 0;
	}
	if (got_rgb) {
		tmp = rgb_front;
		rgb_front = rgb_mid;
		rgb_mid = tmp;
		got_rgb = 0;
	}


	pthread_mutex_unlock(&gl_backbuf_mutex);	
	return true;
}




KinectControl* getKinectControlRef() {
	return kinectControlRef;
}