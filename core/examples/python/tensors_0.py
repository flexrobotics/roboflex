import time
import numpy as np
import roboflex.core.python as rfc

frequency_generator = rfc.FrequencyGenerator(2.0)
tensor_creator = rfc.MapFun(lambda m: {"t": np.ones((2, 3)) * m.message_counter})
message_printer = rfc.MessagePrinter("MESSAGE IS:")
tensor_printer = rfc.CallbackFun(lambda m: print("TENSOR IS:\n", m["t"]))

frequency_generator > tensor_creator > message_printer > tensor_printer;

frequency_generator.start();

time.sleep(3);
    
frequency_generator.stop();

print("DONE")
