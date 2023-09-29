import time
import numpy as np
from roboflex.core import FrequencyGenerator, MapFun, MessagePrinter, CallbackFun
from roboflex.transport.zmq import ZMQContext, ZMQPublisher, ZMQSubscriber

frequency_generator = FrequencyGenerator(2.0)
tensor_creator = MapFun(lambda m: {"t": np.ones((2, 3)) * m.message_counter})
message_printer1 = MessagePrinter("SENDING MESSAGE:")
message_printer2 = MessagePrinter("RECEIVED MESSAGE:")
tensor_printer = CallbackFun(lambda m: print("TENSOR IS:", type(m["t"]), m["t"].shape, m["t"].dtype, "\n", m["t"]))

zmq_context = ZMQContext()
zmq_pub = ZMQPublisher(zmq_context, "inproc://testitpy")
zmq_sub = ZMQSubscriber(zmq_context, "inproc://testitpy")

frequency_generator > tensor_creator > message_printer1 > zmq_pub
zmq_sub > message_printer2 > tensor_printer

frequency_generator.start()
zmq_sub.start()

time.sleep(3)
    
frequency_generator.stop()
zmq_sub.stop()

print("DONE")
