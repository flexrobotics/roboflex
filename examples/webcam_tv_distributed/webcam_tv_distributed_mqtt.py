import time
import roboflex.core as rcc
import roboflex.webcam_uvc as rcw
import roboflex.visualization as rcv
import roboflex.transport.mqtt as rcq

print('DEVICES:', rcw.get_device_list())

WIDTH=800
HEIGHT=600
FPS=60
FORMAT=rcw.uvc_frame_format.UVC_FRAME_FORMAT_MJPEG

mqtt_context = rcq.MQTTContext()

sensor = rcw.WebcamSensor(
    width=WIDTH,
    height=HEIGHT,
    fps=FPS,
    device_index=1,
    format=FORMAT,
    emit_rgb=False,
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

MQTT_BROKER = "127.0.0.1"
MQTT_PORT = 1883
MQTT_TOPIC = "mycamtopic"
pub = rcq.MQTTPublisher(mqtt_context, MQTT_BROKER, MQTT_PORT, MQTT_TOPIC)
sub = rcq.MQTTSubscriber(mqtt_context, MQTT_BROKER, MQTT_PORT, MQTT_TOPIC)

sensor > pub
sub > rcw.WebcamRawToRGBConverter() > viewer

sensor.start()
sub.start()
viewer.start()

time.sleep(60)

sensor.stop()
sub.stop()
viewer.stop()
