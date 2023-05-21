import time
import roboflex.core as rfc
import roboflex.realsense as rfr

print(rfr.RealsenseSensor.get_connected_device_serial_numbers())

config = rfr.Config(
    align_to = rfr.CameraType.RGB,
    rgb_settings = {"fps": 30, "width": 640, "height": 480},
    depth_settings = {"fps": 30, "width": 640, "height": 480},
)
sensor = rfr.RealsenseSensor("827112072758", config)

sensor > rfc.MessagePrinter("GOT:")

sensor.start()

time.sleep(10)

sensor.stop()
