import time
import roboflex.core as rcc
import roboflex.webcam_uvc as rcw
import roboflex.visualization as rcv
import roboflex.transport.zmq as rcz

print('DEVICES:', rcw.get_device_list())

WIDTH=800
HEIGHT=600
FPS=60
FORMAT=rcw.uvc_frame_format.UVC_FRAME_FORMAT_MJPEG
# WIDTH=800
# HEIGHT=600
# FPS=20
# FORMAT=rcw.uvc_frame_format.UVC_FRAME_FORMAT_ANY

zmq_context = rcz.ZMQContext()

sensor = rcw.WebcamSensor(
    width=WIDTH,
    height=HEIGHT,
    fps=FPS,
    device_index=1,
    format=FORMAT,
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

# pub = rcz.ZMQPublisher(zmq_context, "inproc://mycam")
# sub = rcz.ZMQSubscriber(zmq_context, "inproc://mycam")
# pub = rcz.ZMQPublisher(zmq_context, "ipc://mycam")
# sub = rcz.ZMQSubscriber(zmq_context, "ipc://mycam")
pub = rcz.ZMQPublisher(zmq_context, "tcp://*:4567", max_queued_msgs=1)
sub = rcz.ZMQSubscriber(zmq_context, "tcp://127.0.0.1:4567", max_queued_msgs=1)

sensor > pub
sub > viewer

sensor.start()
sub.start()
viewer.start()

time.sleep(10)

sensor.stop()
sub.stop()
viewer.stop()
