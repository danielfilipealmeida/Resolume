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


int main (void)
{
	std::cout << "starting client..."<<std::endl;
	
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REQ);
	
    std::cout << "Connecting to hello world server…" << std::endl;
    socket.connect ("tcp://localhost:5555");
	
    //  Do 10 requests, waiting each time for a response
    for (int request_nbr = 0; request_nbr != 10; request_nbr++) {
		// ask for the depth map
		s_send(socket, "getDepthmap");
		
		// receive result
		std::string requestResult = s_recv(socket);
		
		uint8_t *depthMap = (uint8_t *) strdup(requestResult.c_str());
    }
		 
		
    return 0;
}