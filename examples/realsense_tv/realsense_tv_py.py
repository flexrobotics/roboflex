import time
import roboflex.core as rfc
import roboflex.realsense as rfr
import roboflex.visualization as rcv

config = rfr.Config(
    align_to = rfr.CameraType.RGB,
    rgb_settings = {"fps": 30, "width": 640, "height": 480},
    depth_settings = {"fps": 30, "width": 640, "height": 480},
)
sensor = rfr.RealsenseSensor("827112072758", config)

viewer = rcv.RGBImageTV(
    frequency_hz=24.0, 
    width=640, 
    height=480, 
    image_key="rgb",
    debug=False,
    mirror=True,
)

sensor > viewer

sensor.start()
viewer.start()

time.sleep(10)

sensor.stop()
viewer.stop()
