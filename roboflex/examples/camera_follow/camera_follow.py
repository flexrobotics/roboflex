import time
import roboflex.core as rcc
import roboflex.webcam_uvc as rcw
import roboflex.visualization as rcv
import roboflex.transport.zmq as rcz
import roboflex.metrics_central as rfm
from facedetect import DetectorYoloFaceNode
from pan_tilt_controller import PanTiltController


WIDTH=800
HEIGHT=600
FPS=20
FORMAT=rcw.uvc_frame_format.UVC_FRAME_FORMAT_ANY


profiler = rfm.Profiler()

webcam = rcw.WebcamSensor(
    width=WIDTH,
    height=HEIGHT,
    fps=FPS,
    device_index=1,
    format=FORMAT,
    emit_rgb=True,
)

face_detector = DetectorYoloFaceNode(HEIGHT, WIDTH)

viewer = rcv.RGBImageTV(
    frequency_hz=24.0, 
    width=WIDTH, 
    height=HEIGHT, 
    image_key="rgb",
    debug=False,
    mirror=True,
)

pan_tilt_controller = PanTiltController()

PUB_ADDRESS = "ipc://mycam"
zmq_context = rcz.ZMQContext()
pub = rcz.ZMQPublisher(zmq_context, PUB_ADDRESS, max_queued_msgs=1)
sub1 = rcz.ZMQSubscriber(zmq_context, PUB_ADDRESS, max_queued_msgs=1)

# connect the graph
webcam > pub
sub1 > face_detector > viewer > pan_tilt_controller

# connect all root nodes to the profiler
profiler > pan_tilt_controller
profiler > webcam
profiler > sub1

# start the profiler, which will start all runnable nodes in the graph
profiler.start(profile=True)

try:
    time.sleep(3000)
except KeyboardInterrupt:
    print('Caught Interrupt')

profiler.stop()

print("DONE")
