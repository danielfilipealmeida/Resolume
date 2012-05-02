#include "KinectControl.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*
	KinectControl error codes
 
Code	String
 0		no error.
 1		KinectControl Already Started.
 2		freenect_init() failed.
*/

KinectControl *kinectControlRef = NULL;
uint16_t t_gamma[2048];
int got_rgb = 0;
int got_depth = 0;
volatile int die = 0;
pthread_t freenect_thread;	


pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;


///////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
#pragma mark Other Funcs


void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{
	//printf("Kinect Depth Callback Iteration.\n");
	
	int i;
	uint16_t *depth = (uint16_t*)v_depth;
	
	pthread_mutex_lock(&gl_backbuf_mutex);
	for (i=0; i<640*480; i++) {
		int pval = t_gamma[depth[i]];
		int lb = pval & 0xff;
		switch (pval>>8) {
			case 0:
				kinectControlRef->depth_mid[3*i+0] = 255;
				kinectControlRef->depth_mid[3*i+1] = 255-lb;
				kinectControlRef->depth_mid[3*i+2] = 255-lb;
				break;
			case 1:
				kinectControlRef->depth_mid[3*i+0] = 255;
				kinectControlRef->depth_mid[3*i+1] = lb;
				kinectControlRef->depth_mid[3*i+2] = 0;
				break;
			case 2:
				kinectControlRef->depth_mid[3*i+0] = 255-lb;
				kinectControlRef->depth_mid[3*i+1] = 255;
				kinectControlRef->depth_mid[3*i+2] = 0;
				break;
			case 3:
				kinectControlRef->depth_mid[3*i+0] = 0;
				kinectControlRef->depth_mid[3*i+1] = 255;
				kinectControlRef->depth_mid[3*i+2] = lb;
				break;
			case 4:
				kinectControlRef->depth_mid[3*i+0] = 0;
				kinectControlRef->depth_mid[3*i+1] = 255-lb;
				kinectControlRef->depth_mid[3*i+2] = 255;
				break;
			case 5:
				kinectControlRef->depth_mid[3*i+0] = 0;
				kinectControlRef->depth_mid[3*i+1] = 0;
				kinectControlRef->depth_mid[3*i+2] = 255-lb;
				break;
			default:
				kinectControlRef->depth_mid[3*i+0] = 0;
				kinectControlRef->depth_mid[3*i+1] = 0;
				kinectControlRef->depth_mid[3*i+2] = 0;
				break;
		}
	}
	got_depth++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
	//printf("Kinect RGB Callback Iteration.\n");

	pthread_mutex_lock(&gl_backbuf_mutex);
	
	// swap buffers
	assert (kinectControlRef->rgb_back == rgb);
	kinectControlRef->rgb_back = kinectControlRef->rgb_mid;
	freenect_set_video_buffer(dev, kinectControlRef->rgb_back);
	kinectControlRef->rgb_mid = (uint8_t*)rgb;
	
	got_rgb++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}




// the freenect thread. 
void *freenect_threadfunc(void *arg) {
	printf("Kinect Thread Started.\n");
	
	
	int accelCount = 0;
	
	freenect_set_tilt_degs(kinectControlRef->f_dev,kinectControlRef->freenect_angle);
	freenect_set_led(kinectControlRef->f_dev,LED_RED);
	freenect_set_depth_callback(kinectControlRef->f_dev, depth_cb);
	freenect_set_video_callback(kinectControlRef->f_dev, rgb_cb);
	freenect_set_video_mode(kinectControlRef->f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, kinectControlRef->current_format));
	freenect_set_depth_mode(kinectControlRef->f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	freenect_set_video_buffer(kinectControlRef->f_dev, kinectControlRef->rgb_back);
	
	freenect_start_depth(kinectControlRef->f_dev);
	freenect_start_video(kinectControlRef->f_dev);
	
	//printf("'w'-tilt up, 's'-level, 'x'-tilt down, '0'-'6'-select LED mode, 'f'-video format\n");
	
	while (!die && freenect_process_events(kinectControlRef->f_ctx) >= 0) {
		//printf("Kinect Thread Iteration.\n");
		//Throttle the text output
		if (accelCount++ >= 2000)
		{
			accelCount = 0;
			freenect_raw_tilt_state* state;
			freenect_update_tilt_state(kinectControlRef->f_dev);
			state = freenect_get_tilt_state(kinectControlRef->f_dev);
			double dx,dy,dz;
			freenect_get_mks_accel(state, &dx, &dy, &dz);
			printf("\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", state->accelerometer_x, state->accelerometer_y, state->accelerometer_z, dx, dy, dz);
			fflush(stdout);
		}
		
		if (kinectControlRef->requested_format != kinectControlRef->current_format) {
			freenect_stop_video(kinectControlRef->f_dev);
			freenect_set_video_mode(kinectControlRef->f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, kinectControlRef->requested_format));
			freenect_start_video(kinectControlRef->f_dev);
			kinectControlRef->current_format = kinectControlRef->requested_format;
		}
	}
	
	//printf("\nshutting down streams...\n");
	
	freenect_stop_depth(kinectControlRef->f_dev);
	freenect_stop_video(kinectControlRef->f_dev);
	
	freenect_close_device(kinectControlRef->f_dev);
	freenect_shutdown(kinectControlRef->f_ctx);

	printf("Kinect Thread Finished.\n");
	//printf("-- done!\n");
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
	
	
	kinectControlRef = this;

	//gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
	//gl_frame_cond = PTHREAD_COND_INITIALIZER;
	

	
	// /////////////
	// freenect init
	
	freenect_angle = 0;
	user_device_number = -1;
	

	// freenect formats
	requested_format = FREENECT_VIDEO_RGB;
	current_format = FREENECT_VIDEO_RGB;
	
	
	
	
	// init the bitmaps
	depth_mid = (uint8_t*)malloc(640*480*3);
	depth_front = (uint8_t*)malloc(640*480*3);
	rgb_back = (uint8_t*)malloc(640*480*3);
	rgb_mid = (uint8_t*)malloc(640*480*3);
	rgb_front = (uint8_t*)malloc(640*480*3);
	
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
	


	// init the thread
	/*
	int res;
	res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
	if (res) {
		printf("pthread_create failed\n");
		freenect_shutdown(f_ctx);
		//return 1;
	}
	*/
}



KinectControl::~KinectControl() {
	printf("Destroying KinectControl.\n");
	
	die = 1;
	
	//freenect_shutdown(f_ctx);
	pthread_join(freenect_thread, NULL);
	free(depth_mid);
	free(depth_front);
	free(rgb_back);
	free(rgb_mid);
	free(rgb_front);
	
	kinectControlRef = NULL;
}


int KinectControl::initDevice(unsigned int _user_device_number) {
	printf("KinectControl. initing the device.\n");
	/*
	if ((isInited == true) || (kinectControlRef->isInited == true)) {
		printf("KinectControl already initialized.\n");
		return 1;
	}
	*/
	
	
	user_device_number = _user_device_number;
	if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
		printf("Could not open device\n");
		freenect_shutdown(f_ctx);
		return 1;
	} else {
		printf("Kinect device opened.\n");
		isInited = true;

	}

	
	int res;
	res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
	if (res) {
		printf("pthread_create failed\n");
		freenect_shutdown(f_ctx);
		return 1;
	} else {
		printf("Kinect thread created.\n");
	}

	
	
	return res;
}

void KinectControl::initTextures() {
	
/*
	printf("KinectControl. Initing Textures.\n");
	if ((isInited == true) || (kinectControlRef->isInited == true)) {
		printf("KinectControl already initialized.\n");
		return;
	}
*/
	glEnable(GL_TEXTURE_2D);
	
	glGenTextures(1, &gl_depth_tex);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* temp = (unsigned char*) malloc(640*480*3);
	
	memset(temp, 128, 640*480*3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, temp);
	

	glGenTextures(1, &gl_rgb_tex);
	glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, temp);
	delete temp;
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////
// Other Funcs
#pragma mark Other Funcs





uint8_t *KinectControl::getDepthMid() {
	return kinectControlRef->depth_mid;
}


bool KinectControl::updateData() {
	uint8_t *tmp;
	
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

bool KinectControl::bindDepthTexture() 
{
	/*
	if (depth_front == NULL) 
	{
		printf("KinectControl::bindDepthTexture(). depth_front is NULL.\n");
		return false;
	}
*/
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	//memset(depth_front, 128, 640*480*3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depth_mid);
	return true;
}



KinectControl* getKinectControlRef() {
	return kinectControlRef;
}