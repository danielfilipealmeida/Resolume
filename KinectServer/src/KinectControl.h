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


	
	bool isInited;
	
	int errorCode;
	string errorString;
	
	KinectControl();
	~KinectControl();
	
	int initDevice(unsigned int _user_device_number = 1);
	uint8_t *getDepthMid();
	uint8_t *getRGB();
	
	bool updateData();
	
	
};

