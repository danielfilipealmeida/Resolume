/*
 *  glClient.mm
 *  KinectServerClient
 *
 *  Created by Daniel Almeida on 5/1/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */


#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>


//#include "KinectClient.h"


#include <pthread.h>
#include <zmq.hpp>
#include <string>
#include <iostream>
#include "zhelpers.hpp"




#define textureWidth 64
#define textureHeigth 64



static GLubyte checkTexture[textureWidth][textureHeigth][4];
static GLuint checkTexName, gl_depth_tex;


void makeTextures(void) {
	int i, j, c;
	
	for(i = 0; i < textureHeigth;i++) {
		for(j = 0; j < textureWidth;j++) {
			c = ((((i&0x8)==0)^((j&0x8)) == 0)) * 255;
			checkTexture[i][j][0] = (GLubyte) c;
			checkTexture[i][j][1] = (GLubyte) c;
			checkTexture[i][j][2] = (GLubyte) c;
			checkTexture[i][j][3] = (GLubyte) 255;
		}
	}
	
}


zmq::context_t kinectContext (1);
zmq::socket_t kinectSocket (kinectContext, ZMQ_REQ);


bool	clientThreadLocked;
bool	clientRunning;
pthread_t threadID;
unsigned int depthTextureSize;
uint8_t *depthMap;


void *kinectServerExecLoop(void *arg) {
	while (clientRunning==true) {
		try{
			s_send(kinectSocket, "getDepthmap");
		} catch (zmq::error_t error) {
			std::cout << "Error sending request to Kinect Server" << std::endl;
			std::cout << error.what();
			exit(1);		
		}
		
		std::string requestResult;
		try {
			
			requestResult = s_recv(kinectSocket);
		} catch (zmq::error_t error) {
			std::cout << "Error getting data from Server" << std::endl;
			std::cout << error.what();
			
			exit(1);
		}
		
		clientThreadLocked=true;
		memcpy((void *) depthMap, (void *) requestResult.c_str(), depthTextureSize);
		clientThreadLocked=false;
	}
	free((void *) depthMap);
}


bool initKinectServer() 
{
	if (clientRunning==true) {
		std::cout << "initKinectServer: Kinnect Client Already running." << std::endl;
		return clientRunning;
	}
	
	depthTextureSize = 640*480*3;
	depthMap = (uint8_t*) malloc(depthTextureSize);
	clientRunning = false;
	
	printf("Connecting to the Kinect Server...\n");
	kinectSocket.connect ("tcp://localhost:5555");
	clientRunning=true;
	
	
	// start thread
	pthread_create(&threadID,NULL,kinectServerExecLoop,NULL);
	
	
	return clientRunning;
}

void stopKinectServer() {
	if (clientRunning== false) return;
	clientRunning = false;
	pthread_join(threadID,NULL);
	
}

uint8_t *getKinectDepthMap()
{
	return depthMap;
}



void init(void) {
	glClearColor(0.0,0.0,0.0,0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	
	
	// init generated texture
	makeTextures();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, checkTexName);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeigth, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkTexture);
	
	
	
	// init depth texture
	glGenTextures(1, &gl_depth_tex);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}


void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	//glBindTexture(GL_TEXTURE_2D, checkTexName);
	glColor3f(1.0, 1.0, 1.0);
	
	
	
	//uint8_t *depthMap = getKinectDepthMap();
	if (depthMap!=NULL) {
		glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depthMap);
	}

	
	
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex2f(0.8,0.8);
	glTexCoord2f(0, 1.0);
	glVertex2f(0.8,-0.8);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(-0.8,-0.8);
	glTexCoord2f(1.0, 0);
	glVertex2f(-0.8,0.8);	
	glEnd();
	
	
	/*
	 glBegin(GL_POLYGON);
	 glVertex3f(0.25,0.25,0.0);
	 glVertex3f(0.75,0.25,0.0);
	 glVertex3f(0.75,0.75,0.0);
	 glVertex3f(0.25,0.75,0.0);
	 glEnd();
	 */
	glFlush();
	glDisable(GL_TEXTURE_2D);
}

void reshape(int w, int h) {
	glViewport(0,0,(GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
}


void animate(void) {
	glutPostRedisplay();
}


int main (int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(400, 400);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(animate);
	
	
	// init the server
	initKinectServer();

	
	
	glutMainLoop();
	
	stopKinectServer();
	
	return 0;
}
