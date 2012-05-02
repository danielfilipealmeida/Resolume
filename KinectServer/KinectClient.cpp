/*
 *  KinectClient.cpp
 *  KinectServerClient
 *
 *  Created by Daniel Almeida on 4/29/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "KinectClient.h"

bool	clientThreadLocked;
bool	clientRunning;
pthread_t threadID;
unsigned int depthTextureSize;
uint8_t *depthMap;



zmq::context_t kinectContext (1);
zmq::socket_t kinectSocket (kinectContext, ZMQ_REQ);





void *kinectServerExecLoop(void *arg) {
	while (clientRunning==true) {
		//printf("getting depth image.\n");
		//s_send(kinectSocket, "getDepthmap");
		s_send(kinectSocket, "getRGB");
		std::string requestResult = s_recv(kinectSocket);
		clientThreadLocked=true;
		//depthMap = (uint8_t *) strdup(requestResult.c_str());
		memcpy((void *) depthMap, (void *) requestResult.c_str(), depthTextureSize);
		
		clientThreadLocked=false;
		//free (depthMap);		
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
