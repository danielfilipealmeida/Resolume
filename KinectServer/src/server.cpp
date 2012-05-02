/*!
    @header server.cpp
    @abstract   The Kinect Server For Resolume
    @discussion Serves Kinect data to client aplications
*/

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include "KinectControl.h"
#include "zhelpers.hpp"
#include "math.h"

// Possible Messages: getDepthmap 

KinectControl *kinectControl;


int fps, frameDuration;

bool initKinect() 
{
	std::cout << "Starting Kinnect..." << std::endl;
	kinectControl = new KinectControl();
	if (kinectControl->errorCode != 0) return false;
	kinectControl->initDevice(0);
	if (kinectControl->errorCode != 0) return false;
	
	return true;
}


int main () 
{
	std::cout << "starting server..."<<std::endl;

	fps = 25;
	frameDuration = (int) round(1000.0 / (float) fps);
    
	//  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:5555");
	
	
	// starting kinect
	if (initKinect()==false) {
		std::cout << "Kinect Init Failed. Quiting..." << std::endl;
		return 1;
	}
	
    while (true) {
        
        //  Wait for next request from client
		std::string request = s_recv(socket);
		
		std::cout << "request: "<<request<<std::endl;
		
	

		if (request.compare("getDepthmap") == 0) {
			std::cout<<"sending depth map."<<std::endl;
			uint8_t *depthMap = kinectControl->getDepthMid();
			s_send(socket, (char *) depthMap);
		}
		
		if (request.compare("getRGB") == 0) {
			std::cout<<"sending RGB."<<std::endl;
			uint8_t *rgb = kinectControl->getRGB();
			s_send(socket, (char *) rgb);
		}		
		usleep(frameDuration);		
    }
    return 0;
}