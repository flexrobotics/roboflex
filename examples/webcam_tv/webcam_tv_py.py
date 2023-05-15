import time
import roboflex.core as rcc
import roboflex.webcam_uvc as rcw
import roboflex.visualization as rcv

print('DEVICES:', rcw.get_device_list())

WIDTH=800
HEIGHT=600
FPS=60
FORMAT=rcw.uvc_frame_format.UVC_FRAME_FORMAT_MJPEG
#FORMAT=rcw.uvc_frame_format.UVC_FRAME_FORMAT_ANY

sensor = rcw.WebcamSensor(
    width=WIDTH,
    height=HEIGHT,
    fps=FPS,
    device_index=1,
    format=FORMAT,
    emit_rgb=True,
)
sensor.print_device_info()

viewer = rcv.RGBImageTV(
    frequency_hz=24.0, 
    width=WIDTH, 
    height=HEIGHT, 
    image_key="rgb",
    debug=False,
    mirror=True,
)

sensor > viewer

sensor.start()
viewer.start()

time.sleep(60)

sensor.stop()
viewer.stop()
