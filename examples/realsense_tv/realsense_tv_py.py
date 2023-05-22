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

rgb_viewer = rcv.RGBImageTV(
    frequency_hz=24.0, 
    width=640, 
    height=480, 
    image_key="rgb",
    debug=False,
    mirror=True,
)
depth_viewer = rcv.DepthTV(
    frequency_hz=24.0, 
    width=640, 
    height=480, 
    image_key="depth",
    initial_pos=(720, 0),
    debug=False,
    mirror=True,
)

sensor > rgb_viewer
sensor > depth_viewer

sensor.start()
rgb_viewer.start()
depth_viewer.start()

time.sleep(10)

sensor.stop()
rgb_viewer.stop()
depth_viewer.stop()
