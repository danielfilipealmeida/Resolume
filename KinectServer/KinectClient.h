/*
 *  KinectClient.h
 *  KinectServerClient
 *
 *  Created by Daniel Almeida on 4/29/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __KINECTCLIENT_H__
#define __KINECTCLIENT_H__


#include <pthread.h>
#include <zmq.hpp>
#include <string>
#include <iostream>
#include "zhelpers.hpp"









bool initKinectServer();
void *kinectServerExecLoop(void *arg);
void stopKinectServer();
uint8_t *getKinectDepthMap();

#endif