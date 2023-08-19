import time
import roboflex.core as rcc
import roboflex.webcam_uvc as rcw
import roboflex.visualization as rcv
import roboflex.transport.zmq as rcz
import roboflex.metrics_central as rfm
import roboflex.util.jpeg as ruj

print('DEVICES:', rcw.get_device_list())

# WIDTH=800
# HEIGHT=600
# FPS=60
# FORMAT=rcw.uvc_frame_format.UVC_FRAME_FORMAT_MJPEG
WIDTH=640
HEIGHT=480
FPS=120
FORMAT=rcw.uvc_frame_format.UVC_FRAME_FORMAT_MJPEG

profiler = rfm.Profiler()

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
    mirror=False,
)

# pub = rcz.ZMQPublisher(zmq_context, "inproc://mycam")
# sub = rcz.ZMQSubscriber(zmq_context, "inproc://mycam")
# pub = rcz.ZMQPublisher(zmq_context, "ipc://mycam")
# sub = rcz.ZMQSubscriber(zmq_context, "ipc://mycam")
pub = rcz.ZMQPublisher(zmq_context, "tcp://*:4567", max_queued_msgs=2)
sub = rcz.ZMQSubscriber(zmq_context, "tcp://127.0.0.1:4567", max_queued_msgs=2)

#jpeg_compressor = ruj.JPEGCompressor(image_key="rgb", filename_prefix="./image_", debug=True)
jpeg_compressor = ruj.JPEGCompressor(image_key="rgb", output_key="jpeg", debug=False)
jpeg_decompressor = ruj.JPEGDecompressor(input_key="jpeg", output_key="rgb", debug=False)

# sensor > pub
# sub > viewer

profiler > sensor > jpeg_compressor > pub
profiler > sub > jpeg_decompressor > viewer

# sensor.start()
# sub.start()
# viewer.start()
profiler.start(profile=True)

time.sleep(10000)

# sensor.stop()
# sub.stop()
# viewer.stop()

profiler.stop()
