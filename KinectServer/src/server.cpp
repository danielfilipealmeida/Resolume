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


int main (int argc, char* argv[]) 
{
	string protocol = "ipc";
	string port = "5555";
	string ipcAddress = "ffglkinectfeed";
	string serverAddress;

	// argument handling 
	if (argc>1) 
	{
		if ((strcmp (argv[1], "help")==0) || (strcmp (argv[1], "-h")==0))
		{
			cout << "usage"<<endl;
			cout << "    help, -h               This information"<<endl;
			cout <<	"    <protocol> <port/path> available protocols: tcp, ipc"<<endl;
			cout << "                           Port is related to tcp protocol and path is for ipc."<<endl<<endl;
			
			cout << "examples:"<<endl;
			cout << "    ResolumeKinectServer tcp 5555"<<endl;
			cout << "    ResolumeKinectServer ipc ffglkinectfeed"<<endl<<endl;
			
			cout << "defaults:"<<endl;
			cout << "    protocol: " << protocol<<endl;
			cout << "    port: " << port<<endl;
			cout << "    ipc path: " << ipcAddress<<endl;
			exit(0);
		}
			 
			 
		if ((strcmp (argv[1], "tcp")==0) || (strcmp(argv[1], "ipc")==0)) 
		{
			protocol = argv[1];
			if (argc>2) { 
				if ((strcmp (argv[2], "")>0)) 
				{
					if(strcmp (argv[1], "tcp")==0)
					{
						port = argv[2];
					}
					if(strcmp (argv[1], "ipc")==0)
					{
						ipcAddress = argv[2];
					}
				}
			}
		}
		else 
		{
			cout << "Invalid protocol. Available protocols [tcp/ipc]. Default protocol: " << protocol << "." << std::endl;
			exit(1);
		}
		
		
	}
	
	std::cout << "starting server..."<<std::endl;


	fps = 25;
	frameDuration = (int) round(1000.0 / (float) fps);
    
	//  Prepare our context and socket
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REP);
	
	try {
		if (protocol == "tcp") 
		{
			serverAddress = "tcp://*:" + port;
			cout << "connecting to " << serverAddress << "." << endl;
			//socket.bind ("tcp://*:5555");
			
			socket.bind (serverAddress.c_str());
			
		}
		if (protocol == "ipc") 
		{
			serverAddress = "ipc://"+ipcAddress;
			cout << "connecting to " << serverAddress << "." << endl;
			
			socket.bind (serverAddress.c_str());
		}
		
		
	} catch (zmq::error_t error) {
		std::cout << "Error Binding to address '" << serverAddress << "'." << std::endl;
		std::cout << error.what();
		std::cout << std::endl;
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
		
		s_sleep(frameDuration);
    }
	
	socket.close();
	return 0;
}