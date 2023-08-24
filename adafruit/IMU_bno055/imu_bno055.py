import roboflex.core as rfc

# These must be somewhere. In our case, they
# come from requirements.txt, installed into
# a python virtual environment (pyvenv).
import adafruit_bno055
import board

class I2C_Bus:
    bus = board.I2C()

class IMU_BNO055_Node(rfc.FrequencyGenerator):
    def __init__(self, i2c_bus=None, frequency=100.0, name="IMU_BNO055"):
        super().__init__(frequency, name)
        self.bus = i2c_bus if i2c_bus is not None else I2C_Bus()
        self.sensor = adafruit_bno055.BNO055_I2C(self.bus.bus)

    def on_trigger(self, t):
        self.signal({'q': self.sensor.quaternion})
