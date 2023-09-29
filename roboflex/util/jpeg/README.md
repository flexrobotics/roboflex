# roboflex.util.jpeg

Roboflex support for jpeg compression to file and memory, and jpeg decompression from memory.

Useful for compressing images over slow transports (wifi, etc).

## System Dependencies

    None! We build jpeg-compression from source...

## Nodes

    JPEGCompressor
    JPEGDecompressor

## Examples

See [../../examples/webcam_tv_distributed/webcam_tv_distributed_zmq.py](../../examples/webcam_tv_distributed/webcam_tv_distributed_zmq.py)

    $ bazel run -c opt //roboflex/examples/webcam_tv_distributed:webcam_tv_distributed_zmq