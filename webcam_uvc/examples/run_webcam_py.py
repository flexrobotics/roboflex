import time
import roboflex.core as rcc
import roboflex.webcam_uvc as rcw

print('DEVICES:', rcw.get_device_list())

WIDTH=800
HEIGHT=600
FPS=60

sensor = rcw.WebcamSensor(
    width=WIDTH,
    height=HEIGHT,
    fps=FPS,
    device_index=1,
    format=rcw.uvc_frame_format.UVC_FRAME_FORMAT_MJPEG,
    #format=rcw.uvc_frame_format.UVC_FRAME_FORMAT_ANY,
)
sensor.print_device_info()

sensor > rcc.MessagePrinter("Received")

sensor.start()

time.sleep(10)

sensor.stop()
