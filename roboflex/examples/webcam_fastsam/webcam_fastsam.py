import time
import numpy as np
import torch
import roboflex.core as rcc
import roboflex.webcam_uvc as rcw
import roboflex.visualization as rcv
import roboflex.transport.tq as rctq
import roboflex.metrics_central as rfm
from submodules.FastSAM.fastsam import FastSAM, FastSAMPrompt


# A class that can get the masks from FastSAM
# given some image tensor. Nothing fancy, just
# a wrapper for convenience.
class FastSAMMasker:

    def __init__(self, height, width, device='cuda'):
        self.height = height 
        self.width = width
        self.device = device
        self.model = FastSAM('examples/webcam_fastsam/models/FastSAM-x.pt')

    def get_masks(self, image_tensor):
        everything_results = self.model(image_tensor, device=self.device, retina_masks=True, imgsz=self.width, conf=0.4, iou=0.9,)
        prompt_process = FastSAMPrompt.from_np_tensor(image_tensor, everything_results, device=self.device)
        masks = prompt_process.everything_prompt()
        plotted_result = prompt_process.plot_to_result(annotations=masks, better_quality=False, withContours=False)
        return plotted_result
    
    
# Inherit the roboflex.Node class, that can receive an
# rgb tensor, get the masks, and signal that, using an 
# instance of the FastSAMMasker class.
class FastSAMNode(rcc.Node):

    def __init__(self, height, width, device='cuda'):
        super().__init__("FastSAMNode")
        self.masker = FastSAMMasker(height, width, device)

    def receive(self, m):
        rgb = m["rgb"] # (H, W, 3) uint8
        mask_plotted_rgb = self.masker.get_masks(rgb)
        self.signal({"rgb": mask_plotted_rgb}) 


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

masker = FastSAMNode(HEIGHT, WIDTH)

viewer1 = rcv.RGBImageTV(
    frequency_hz=24.0, 
    width=WIDTH, 
    height=HEIGHT, 
    image_key="rgb",
    debug=False,
    mirror=False,
    name='Webcam',
)

viewer2 = rcv.RGBImageTV(
    frequency_hz=24.0, 
    width=WIDTH, 
    height=HEIGHT, 
    image_key="rgb",
    debug=False,
    mirror=False,
    name='FastSAM',
)

tq1 = rctq.TQPubSub("tq1", max_queued_msgs=1)
tq2 = rctq.TQPubSub("tq2", max_queued_msgs=1)

webcam > tq1 > viewer1
webcam > tq2 > masker > viewer2
profiler > webcam

# start the profiler, which will start all runnable nodes in the graph
profiler.start(profile=True)

try:
    time.sleep(600)
except KeyboardInterrupt:
    print('Caught Interrupt')

profiler.stop()

print("DONE")
