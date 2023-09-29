# roboflex.webcam_uvc

Support for web cameras on linux - actually just a thin wrapper around libuvc, so if your camera supports libuvc, you should be good to go.

## System dependencies

    apt-get install libusb-1.0-0-dev
    apt-get install libjpeg-dev

## Import

    import numpy
    import roboflex.core as rcc
    import roboflex.webcam_uvc as rcw

## Nodes

There is only one: **WebcamSensor**

    # all parameters optional: below are the defaults
    webcam_sensor = rcw.WebcamSensor(
        width,
        height,
        fps,
        device_index = -1,
        format = UVC_FRAME_FORMAT_ANY,
        name = "WebcamSensor",
    )

    # must be started!
    webcam_sensor.start()

    # you can do this:
    webcam_sensor.print_device_info()

## Messages

    from roboflex.webcam_uvc import WebcamDataRGB

API:

    # the timestamp just before reading from device
    message.t0 -> Float

    # the timestamp just after reading from device
    message.t1 -> Float

    # the capture time from the device
    message.capture_time -> Float

    # the sequence number from the device
    message.sequence -> Int

    # numpy array of shape=(width, height, channels), dtype=short
    message.rgb -> np.ndarray

DYNOFLEX:

    # the timestamp just before reading from device
    message["t0"] -> Double

    # the timestamp just after reading from device
    message["t1"] -> Double

    # the capture time from the device
    message["t"] -> Double

    # the sequence number from the device
    message["s"] -> Double

    # numpy array of shape=(width, height, channels), dtype=short
    message["rgb"] -> np.ndarray


## Other


Description of a device:

    from roboflex.webcam_uvc import DeviceDescriptor

    dd.idVendor -> int
    dd.idProduct -> int
    dd.bcdUVC -> int
    dd.serialNumber -> str
    dd.manufacturer -> str
    dd.product -> str

    # you probably just want to print it:
    str(dd)


Free function: get list of connected devices (webcams) - list of DeviceDescriptor, above.

    get_device_list() -> [DeviceDescriptor]

Available frame formats: pass to constructor of WebcamSensor for the format parameter.

    from roboflex.webcam_uvc import uvc_frame_format

    uvc_frame_format.UVC_FRAME_FORMAT_ANY
    uvc_frame_format.UVC_FRAME_FORMAT_UNCOMPRESSED
    uvc_frame_format.UVC_FRAME_FORMAT_COMPRESSED

    # YUYV/YUV2/YUV422: YUV encoding with one luminance value per pixel and
    # one UV (chrominance) pair for every two pixels.
    uvc_frame_format.UVC_FRAME_FORMAT_YUYV
    uvc_frame_format.UVC_FRAME_FORMAT_UYVY

    # 24-bit RGB
    uvc_frame_format.UVC_FRAME_FORMAT_RGB
    uvc_frame_format.UVC_FRAME_FORMAT_BGR

    # Motion-JPEG (or JPEG) encoded images
    uvc_frame_format.UVC_FRAME_FORMAT_MJPEG
    uvc_frame_format.UVC_FRAME_FORMAT_H264

    # Greyscale images
    uvc_frame_format.UVC_FRAME_FORMAT_GRAY16
    uvc_frame_format.UVC_FRAME_FORMAT_GRAY8

    # Raw colour mosaic images
    uvc_frame_format.UVC_FRAME_FORMAT_BY8
    uvc_frame_format.UVC_FRAME_FORMAT_BA81
    uvc_frame_format.UVC_FRAME_FORMAT_SGRBG8
    uvc_frame_format.UVC_FRAME_FORMAT_SGBRG8
    uvc_frame_format.UVC_FRAME_FORMAT_SRGGB8
    uvc_frame_format.UVC_FRAME_FORMAT_SBGGR8

    # YUV420: NV12
    uvc_frame_format.UVC_FRAME_FORMAT_NV12

    # Number of formats understood
    uvc_frame_format.UVC_FRAME_FORMAT_COUNT


## Linux webcam help

Access denied to your webcam?

First, list your usb devices and find your webcam:

    lsusb
    
For me, I see:

    Bus 002 Device 025: ID 32e4:9230 HD USB Camera HD USB Camera

create a file called '50-usb-webcam.rules' in /etc/udev/rules.d with a single line:

    SUBSYSTEM=="usb", ATTR{idVendor}="HD USB Camera", ATTR{idProduct}="HD USB Camera", MODE="0666"

then

    udevadm control --reload-rules && udevadm trigger