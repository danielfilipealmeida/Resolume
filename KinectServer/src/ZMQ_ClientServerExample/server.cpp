#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>


int main () {
	std::cout << "starting server..."<<std::endl;

    
	//  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:5555");
	
    while (true) {
        zmq::message_t request;
		
        //  Wait for next request from client
        socket.recv (&request);
		std::cout << "Received Hello" << std::endl;
		
		sleep(1);
		
		zmq::message_t reply (5);
		memcpy((void *) reply.data(), "World" , 5);
		socket.send (reply);
		
		
    }
    return 0;
}