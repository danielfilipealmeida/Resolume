/*
 *  KinectClient.cpp
 *  KinectServerClient
 *
 *  Created by Daniel Almeida on 4/29/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include <pthread.h>
#include <string>
#include <zmq.hpp>
#include "zhelpers.hpp"
#include "math.h"
#include "KinectClient.h"

uint8_t kinectClientMode;
bool clientThreadLocked;
bool clientRunning;
pthread_t threadID;
unsigned int depthTextureSize;
uint8_t *depthMap;
uint8_t *rgb;
int fps, frameDuration;
std::string			protocol, serverAddress;
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

zmq::context_t *kinectContext;
zmq::socket_t *kinectSocket;

void cleanUp() 
{
	if (kinectContext!=NULL) 
		delete(kinectContext);
	if (kinectSocket!=NULL) 
		delete(kinectSocket);
}

void *kinectServerExecLoop(void *arg) 
{
	s_catch_signals ();
	while (clientRunning) 
	{
		clientThreadLocked = true;
		pthread_mutex_lock(&mymutex);
		if (kinectClientMode == 0) 
		{
			s_send(*kinectSocket, "getDepthmap");
			std::string requestResult = s_recv(*kinectSocket);
			memcpy((void *) depthMap, (void *) requestResult.c_str(), depthTextureSize);
		}
		
		if (kinectClientMode == 1) 
		{
			s_send(*kinectSocket, "getRGB");
			std::string requestResult = s_recv(*kinectSocket);
			memcpy((void *) rgb, (void *) requestResult.c_str(), depthTextureSize);
		}
		
		clientThreadLocked = false;
		pthread_mutex_unlock(&mymutex);
		s_sleep(frameDuration);

	}
	zmq_close(kinectSocket);
	zmq_term(kinectContext);
	free((void *) rgb);
	free((void *) depthMap);
	pthread_exit(NULL);
}


bool initKinectServer(std::string _protocol, std::string _port, std::string ipcAddress) 
{
	if (clientRunning) 
	{
		std::cout << "initKinectServer: Kinnect Client Already running." << std::endl;
		return clientRunning;
	}
	
	if (_protocol == "tcp") 
	{
	
		serverAddress = _protocol + "://localhost:" + _port;
	}
	if (_protocol == "ipc") 
	{
		serverAddress = "ipc://"+ipcAddress;
	}
	
	kinectContext = new zmq::context_t(1);
	kinectSocket = new zmq::socket_t(*kinectContext, ZMQ_REQ);
	
	kinectClientMode = 0;
	fps = 25;
	frameDuration = (int) round(1000.0 / (float) fps);
	
	depthTextureSize = 640*480*3;
	depthMap = (uint8_t*) malloc(depthTextureSize);
	rgb = (uint8_t*) malloc(depthTextureSize);
	clientRunning = false;
	
	printf("Connecting to the Kinect Server at %s...\n", serverAddress.c_str());
	try 
	{
		//kinectSocket->connect ("tcp://localhost:5555");
		kinectSocket->connect (serverAddress.c_str());
		clientRunning=true;
	} catch (zmq::error_t error) {
		std::cout << "Error Binding to address" << serverAddress  << std::endl;
		std::cout << error.what();
		cleanUp();
		exit(1);		
	}
	
	// start thread
	int res = pthread_create(&threadID,NULL,kinectServerExecLoop,NULL);
	if (res!=0) 
	{
		std::cout << "Error starting FFGLKinect Thread"<<std::endl;
		cleanUp();
		exit(1);
	}
	
	return clientRunning;
}

void stopKinectServer() 
{
	if (!clientRunning) 
		return;
	pthread_mutex_lock(&mymutex);
	clientRunning = false;
	pthread_mutex_unlock(&mymutex);
	//pthread_exit(&threadID);
	pthread_join(threadID,NULL);
	//cleanUp();
}

uint8_t *getKinectDepthMap()
{
	return depthMap;
}

uint8_t *getKinectRGB()
{
	return rgb;
}

void setKinectMode(uint8_t _kinectMode)
{
	kinectClientMode = _kinectMode;
}