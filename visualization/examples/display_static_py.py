import time
import numpy as np
from roboflex.core import FrequencyGenerator, MapFun
from roboflex.visualization import RGBImageTV

frequency_generator = FrequencyGenerator(30.0)
random_tensor_creator = MapFun(lambda m: {"rgb": np.random.randint(0, 255, (480, 640, 3), np.uint8)})
visualizer = RGBImageTV(24.0, 640, 480)

frequency_generator > random_tensor_creator > visualizer

frequency_generator.start()
visualizer.start()

time.sleep(5)

frequency_generator.stop()
visualizer.stop()

print("DONE")