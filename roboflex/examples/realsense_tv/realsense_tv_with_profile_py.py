import time
import roboflex.core as rfc
import roboflex.realsense as rfr
import roboflex.visualization as rcv
import roboflex.metrics_central as rfm

WIDTH = 848
HEIGHT = 480
FPS = 60

profiler = rfm.Profiler()

config = rfr.Config(
    align_to = rfr.CameraType.RGB,
    rgb_settings = {"fps": FPS, "width": WIDTH, "height": HEIGHT},
    depth_settings = {"fps": FPS, "width": WIDTH, "height": HEIGHT},
)
sensor = rfr.RealsenseSensor("827112072758", config)

rgb_viewer = rcv.RGBImageTV(
    frequency_hz=24.0, 
    width=WIDTH, 
    height=HEIGHT, 
    image_key="rgb",
    debug=False,
    mirror=True,
)

depth_viewer = rcv.DepthTV(
    frequency_hz=24.0, 
    width=WIDTH, 
    height=HEIGHT, 
    image_key="depth",
    initial_pos=(848+50, 0),
    debug=False,
    mirror=True,
)

profiler > sensor >> [rgb_viewer, depth_viewer]

profiler.start(profile=True)

time.sleep(1000)

profiler.stop()
