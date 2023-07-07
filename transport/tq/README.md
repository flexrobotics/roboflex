# roboflex.transport.tq

A thread-safe queue. Can only be used to communicate from one thread to one other thread.

Instantiate a TQPubSub:

    unsigned int queue_size = 20;
    unsigned int timeout_ms = 10;
    tq1 = TQPubSub("tq1", queue_size, timeout_ms);

Or in python:

    tq1 = TQPubSub("tq1", max_queued_msgs=20, timeout_milliseconds=10)

Connect it both to the producer and receiver:

    maybe_a_webcam > tq1 > viewer

Start the queue: 

    tq1.start()

Now tq1 and its downstream nodes will run in a separate thread. You can't get much lower latency that this...