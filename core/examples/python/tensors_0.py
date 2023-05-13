import time
import numpy as np
from roboflex.core import FrequencyGenerator, MapFun, MessagePrinter, CallbackFun

frequency_generator = FrequencyGenerator(2.0)
tensor_creator = MapFun(lambda m: {"t": np.ones((2, 3)) * m.message_counter})
message_printer = MessagePrinter("MESSAGE IS:")
tensor_printer = CallbackFun(lambda m: print("TENSOR IS:", type(m["t"]), m["t"].shape, m["t"].dtype, "\n", m["t"]))

frequency_generator > tensor_creator > message_printer > tensor_printer;

frequency_generator.start();

time.sleep(3);
    
frequency_generator.stop();

print("DONE")
