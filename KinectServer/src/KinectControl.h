/*
 *  KinectControl.h
 *  FFGLPlugins
 *
 *  Created by Daniel Almeida on 4/9/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */


#include "libfreenect.h"

#if defined(_MSC_VER) || defined(_WIN32) || defined(WIN32) || defined(__MINGW32__)
// do windows stuff
#else
// mac and linux need this
#include <libusb.h>
#endif


// includes for STL
#include <iostream>
#include <map>
#include <string>


#if defined(__APPLE__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif


using namespace std;




class KinectControl {

public:
	
	// freenect thread variables


	
	// freenect config vars
	freenect_context *f_ctx;
	freenect_device *f_dev;
	freenect_video_format requested_format;
	freenect_video_format current_format;
	
	
	int freenect_angle;
	int freenect_led;
	unsigned int user_device_number;
	
	// opengl stuff. textures...
	GLuint gl_depth_tex;
	GLuint gl_rgb_tex;

	
	// back: owned by libfreenect (implicit for depth)
	// mid: owned by callbacks, "latest frame ready"
	// front: owned by GL, "currently being drawn"
	uint8_t *depth_mid, *depth_front;
	uint8_t *rgb_back, *rgb_mid, *rgb_front;
	
	
	bool isInited;
	
	int errorCode;
	string errorString;
	
	KinectControl();
	~KinectControl();
	
	int initDevice(unsigned int _user_device_number = 1);
	void initTextures();
	uint8_t *getDepthMid();
	
	bool updateData();
	bool bindDepthTexture();
	
	
};

