import roboflex.core as rcc
from yoloface.face_detector import YoloDetector

# uses https://github.com/elyha7/yoloface, cloned into yoloface

# Really just a wrapper around the YoloDetector class.
class DetectorYoloFace:

    def __init__(self, height, width):
        self.height = height 
        self.width = width
        self.model = YoloDetector(device="cuda:0", min_face=90)

    def calculate_face_center(self, rgb):
        bboxes, _ = self.model(rgb) # <class 'list'> [[[343, 121, 739, 595]]]
        if len(bboxes) > 0 and len(bboxes[0]) > 0:

            # compute the center of the face as a fraction of my rectangle
            x1, y1, x2, y2 = bboxes[0][0]
            y1 = min(y1, self.height-1)
            y2 = min(y2, self.height-1)
            x1 = min(x1, self.width-1)
            x2 = min(x2, self.width-1)
            x = (x1 + x2) * 0.5 / self.width
            y = (y1 + y2) * 0.5 / self.height

            # draw the rect in green
            rgb = rgb.copy()
            color = [0,255,0]
            w = 2
            rgb[y1:y1+w,x1:x2] = color
            rgb[y2:y2+w,x1:x2] = color
            rgb[y1:y2,x1:x1+w] = color
            rgb[y1:y2,x2:x2+w] = color

            # return the center x and y, and the new rgb-with-green-rect image
            return x, y, rgb

        else:
            return -1.0, -1.0, rgb

# This is a node that wraps the DetectorYoloFace class.
# All it really does is receive an rgb image, call the
# DetectorYoloFace class, and then signal the x, y, and
# rgb values.
class DetectorYoloFaceNode(rcc.Node):

    def __init__(self, height, width):
        super().__init__("facedetectoryolo")
        self.detecter = DetectorYoloFace(height, width)

    def receive(self, m):
        rgb = m["rgb"] # (H, W, 3) uint8
        x, y, rgb = self.detecter.calculate_face_center(rgb)
        self.signal({"x": x, "y": y, "rgb": rgb})
