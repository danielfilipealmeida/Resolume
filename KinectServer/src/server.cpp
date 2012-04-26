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

// Possible Messages: getDepthmap 

KinectControl *kinectControl;


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
			
			//s_send(socket, "here goes the depth map");
			uint8_t *depthMap = kinectControl->getDepthMid();
			depthMap[0]=10;
			s_send(socket, (char *) depthMap);
			
		}
		sleep(1);
		
		
		
    }
    return 0;
}