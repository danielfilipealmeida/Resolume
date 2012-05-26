//
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <iostream>
#include "zhelpers.hpp"



zmq::context_t *context;
zmq::socket_t *socket;



void cleanUp() {
	if (context!=NULL) delete(context);
	if (socket!=NULL) delete(socket);
}



int main (void)
{
	std::cout << "starting client..."<<std::endl;
	
    //  Prepare our context and socket
	context = new zmq::context_t(1);
    socket = new zmq::socket_t(*context, ZMQ_REQ);
	
    std::cout << "Connecting to hello world server…" << std::endl;
	
	try {
		socket->connect ("tcp://localhost:5555");
	}
	catch (zmq::error_t error) {
		std::cout << "Error connecting to address " << std::endl;
		std::cout << error.what();
		cleanUp();
		exit(1);
	}
    //  Do 10 requests, waiting each time for a response
    for (int request_nbr = 0; request_nbr != 10; request_nbr++) {
		// ask for the depth map
		s_send(*socket, "getDepthmap");
		
		// receive result
		std::string requestResult = s_recv(*socket);
		
		uint8_t *depthMap = (uint8_t *) strdup(requestResult.c_str());
    }
		 
	cleanUp();
    return 0;
}