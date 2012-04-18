
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <zmq.hpp>

void *worker_routine (void *arg)
{
    //  This is the body of the worker thread(s).

    zmq::context_t *ctx = (zmq::context_t*) arg;

    //  Worker thread is a 'replier', i.e. it receives requests and returns
    //  replies.
    zmq::socket_t s (*ctx, ZMQ_REP);

    //  Connect to the dispatcher (queue) running in the main thread.
    s.connect ("inproc://workers");

    while (true) {

        //  Get a request from the dispatcher.
        zmq::message_t request;
        s.recv (&request);

        //  Our server does no real processing. So let's sleep for a while
        //  to simulate actual processing.
        sleep (1);

        //  Send the reply. No point in filling the data in as the client
        //  is a dummy and won't check it anyway.
        zmq::message_t reply (10);
        memset (reply.data (), 0, reply.size ());
        s.send (reply);
    }
}

int main ()
{
    //  One I/O thread in the thread pool will do.
    zmq::context_t ctx (1);

    //  Create an endpoint for worker threads to connect to.
    //  We are using XREQ socket so that processing of one request
    //  won't block other requests.
    zmq::socket_t workers (ctx, ZMQ_XREQ);
    workers.bind ("inproc://workers");

    //  Create an endpoint for client applications to connect to.
    //  We are usign XREP socket so that processing of one request
    //  won't block other requests.
    zmq::socket_t clients (ctx, ZMQ_XREP);
    clients.bind ("tcp://lo:5555");

    //  We'll use 11 application threads. Main thread will be used to run
    //  'dispatcher' (queue). Aside of that there'll be 10 worker threads.
    //  Launch 10 worker threads.
    for (int i = 0; i != 10; i++) {
        pthread_t worker;
        int rc = pthread_create (&worker, NULL, worker_routine, (void*) &ctx);
        assert (rc == 0);
    }

    //  Use queue device as a dispatcher of messages from clients to worker
    //  threads.
    zmq::device (ZMQ_QUEUE, clients, workers);

    return 0;
}
