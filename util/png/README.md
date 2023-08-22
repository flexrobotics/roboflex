# roboflex.util.png

Roboflex support for png compression to file and memory, and png decompression from memory.

Useful for compressing images over slow transports (wifi, etc).

## System Dependencies

    None! We build lodepng from source...

## Nodes

    PNGCompressor
    PNGDecompressor

## Examples

See [../../examples/webcam_tv_distributed/webcam_tv_distributed_zmq.py](../../examples/webcam_tv_distributed/webcam_tv_distributed_zmq.py)

    $ bazel run -c opt //examples/webcam_tv_distributed:webcam_tv_distributed_zmq