import time
import numpy as np
from roboflex.core import Node, FrequencyGenerator, MapFun, MessagePrinter, CallbackFun


# This example shows how to create a graph of nodes that pass messages containing
# numpy tensors (that can be interpreted at the c++-level as xtensor or eigen 
# objects) between each other in a chain.


# ----------- 
# create nodes of the graph

# The first node will signal at 2 hz. FrequencyGenerator is a library node that
# simply signals a BlankMessage at a given frequency. It runs in a thread, and must
# be started and stopped.
frequency_generator = FrequencyGenerator(2.0)

# Next, this node creates a message containing a numpy tensor. This python dict 
# will be encapsulated into a DynoFlex message, and be passed to the next node in 
# the graph.
tensor_creator = MapFun(lambda m: {"t": np.ones((2, 3)) * m.message_counter})

# These nodes print stuff out.
message_printer = MessagePrinter("MESSAGE IS:")

# Node is designed for subclassing.
class MyTensorPrinter(Node):
    def receive(self, m):
        print("MY TENSOR IS:", type(m["t"]), m["t"].shape, m["t"].dtype, "\n", m["t"])

tensor_printer = MyTensorPrinter()

# ----------- 
# connect nodes of the graph. It's easy to distribute the graph into
# multiple cpus using nodes in roboflex/transport.
frequency_generator > tensor_creator > message_printer > tensor_printer

# ----------- 
# start the root node (the other nodes, in this case, will run in the root node's thread).
frequency_generator.start()

# ----------- 
# go for a while
time.sleep(3)
    
# ----------- 
# stop the root node
frequency_generator.stop()

print("DONE")
