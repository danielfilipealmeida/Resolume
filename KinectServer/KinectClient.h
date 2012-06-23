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

#include <iostream>
#include <string>

bool initKinectServer(std::string _protocol="ipc", std::string _port="5555", std::string ipcAddress="ffglkinectfeed");
void *kinectServerExecLoop(void *arg);
void stopKinectServer();
uint8_t *getKinectDepthMap();
uint8_t *getKinectRGB();
void setKinectMode(uint8_t _kinectMode);


#endif