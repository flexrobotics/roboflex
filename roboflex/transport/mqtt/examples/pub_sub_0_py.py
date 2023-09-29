import time
import numpy as np
from roboflex.core import FrequencyGenerator, MapFun, MessagePrinter, CallbackFun
from roboflex.transport.mqtt import MQTTContext, MQTTPublisher, MQTTSubscriber

frequency_generator = FrequencyGenerator(2.0)
tensor_creator = MapFun(lambda m: {"t": np.ones((2, 3)) * m.message_counter})
message_printer1 = MessagePrinter("SENDING MESSAGE:")
message_printer2 = MessagePrinter("RECEIVED MESSAGE:")
tensor_printer = CallbackFun(lambda m: print("TENSOR IS:", type(m["t"]), m["t"].shape, m["t"].dtype, "\n", m["t"]))

mqtt_context = MQTTContext()
mqtt_pub = MQTTPublisher(mqtt_context, "127.0.0.1", 1883, "tensortopic1")
mqtt_sub = MQTTSubscriber(mqtt_context, "127.0.0.1", 1883, "tensortopic1")

frequency_generator > tensor_creator > message_printer1 > mqtt_pub
mqtt_sub > message_printer2 > tensor_printer

frequency_generator.start()
mqtt_sub.start()

time.sleep(3)
    
frequency_generator.stop()
mqtt_sub.stop()

print("DONE")
