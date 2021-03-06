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
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REP);
	
	try {
		socket.bind ("tcp://*:5555");
	} catch (zmq::error_t error) {
		std::cout << "Error Binding to address" << std::endl;
		std::cout << error.what();
	
		exit(1);
	}
	
	// starting kinect
	if (initKinect()==false) {
		std::cout << "Kinect Init Failed. Quiting..." << std::endl;
		
		exit(2);
	}
	
	
	int bufferSize = 3*640*480;
	char *depth=(char *)malloc(bufferSize);
	memset(depth, 255, bufferSize);
	kinectControl->setDepthMid(depth);
	
    while (true) {
        
        //  Wait for next request from client
		std::string request;
		try {
			request = s_recv(socket);
		}
		catch (zmq::error_t error) {
			std::cout << "Error recieving message." << std::endl;
			std::cout << error.what();
			exit(1);
		}
		
		if (request.compare("getRGB") == 0) {
			try {
				uint8_t *rgb = kinectControl->getRGB();
				zmq::message_t message(bufferSize);
				memcpy(message.data(), rgb, bufferSize);
				socket.send(message);
			}
			catch (zmq::error_t error) {
				std::cout << "Error sending RGB image." << std::endl;
				std::cout << error.what();
				exit(1);				
			}
			/*
			uint8_t *rgb = kinectControl->getRGB();
			s_send(socket, (char *) rgb);
			 */
		}	
		
		if (request.compare("getDepthmap") == 0) {		
			try {
				zmq::message_t message(bufferSize);
				memcpy(message.data(), depth, bufferSize);
				socket.send(message);
			}
			catch (zmq::error_t error) {
				std::cout << "Error sending depth image." << std::endl;
				std::cout << error.what();
				exit(1);
				
			}
			
		}		
		
		//usleep(frameDuration);	
		s_sleep(frameDuration);
    }
	
	socket.close();
	return 0;
}