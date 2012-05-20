/*
 *  KinectClient.cpp
 *  KinectServerClient
 *
 *  Created by Daniel Almeida on 4/29/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "KinectClient.h"

uint8_t kinectClientMode;

bool	clientThreadLocked;
bool	clientRunning;
pthread_t threadID;
unsigned int depthTextureSize;
uint8_t *depthMap;
uint8_t *rgb;


zmq::context_t *kinectContext;
zmq::socket_t *kinectSocket;


void cleanUp() {
	if (kinectContext!=NULL) delete(kinectContext);
	if (kinectSocket!=NULL) delete(kinectSocket);
}




void *kinectServerExecLoop(void *arg) {
	while (clientRunning==true) {
		clientThreadLocked=true;

		if (kinectClientMode == 0) {
			s_send(*kinectSocket, "getDepthMap");
			std::string requestResult = s_recv(*kinectSocket);
			memcpy((void *) depthMap, (void *) requestResult.c_str(), depthTextureSize);
		}
		
		if (kinectClientMode == 1) {
			s_send(*kinectSocket, "getRGB");
			std::string requestResult = s_recv(*kinectSocket);
			memcpy((void *) rgb, (void *) requestResult.c_str(), depthTextureSize);
		}
		
		clientThreadLocked=false;
		//free (depthMap);		
	}
	free((void *) rgb);
	free((void *) depthMap);
}


bool initKinectServer() 
{
	if (clientRunning==true) {
		std::cout << "initKinectServer: Kinnect Client Already running." << std::endl;
		return clientRunning;
	}
	
	
	kinectContext = new zmq::context_t(1);
	kinectSocket =new zmq::socket_t(*kinectContext, ZMQ_REQ);
	
	kinectClientMode=1;

	
	depthTextureSize = 640*480*3;
	depthMap = (uint8_t*) malloc(depthTextureSize);
	rgb = (uint8_t*) malloc(depthTextureSize);
	clientRunning = false;
	
	printf("Connecting to the Kinect Server...\n");
	try {
		kinectSocket->connect ("tcp://localhost:5555");
		clientRunning=true;
	}
	catch (zmq::error_t error) {
		std::cout << "Error Binding to address" << std::endl;
		std::cout << error.what();
		cleanUp();
		exit(1);		
	}
	
	// start thread
	pthread_create(&threadID,NULL,kinectServerExecLoop,NULL);
	
	
	return clientRunning;
}

void stopKinectServer() {
	if (clientRunning== false) return;
	clientRunning = false;
	pthread_join(threadID,NULL);
	cleanUp();
}

uint8_t *getKinectDepthMap()
{
	return depthMap;
}


uint8_t *getKinectRGB()
{
	return rgb;
}

void setKinectMode(uint8_t _kinectMode){
	kinectClientMode = _kinectMode;
}