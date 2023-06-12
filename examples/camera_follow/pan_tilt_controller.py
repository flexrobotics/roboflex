import time
import roboflex.core as rcc
import roboflex.transport.zmq as rcz
import roboflex.dynamixel as rcd


class PanTiltController(rcd.DynamixelRemoteController):
    def __init__(self, pantilt_speed = 50.0):
        super().__init__("pan_tilt_controller")

        self.pantilt_speed = pantilt_speed

        self.dynamixel_node = rcd.DynamixelGroupNode(
            controller = rcd.DynamixelGroupController.VelocityController(
                device_name='/dev/ttyUSB0',
                baud_rate=3000000,
                dxl_ids=[5,6],
            ),
        )

        self > self.dynamixel_node
        self.dynamixel_node > self

        self.target_x = -1.0
        self.target_y = -1.0

    def receive(self, m):
        if 'x' in m.keys() and 'y' in m.keys():
            self.target_x = m["x"]
            self.target_y = m["y"]
        else:
            super().receive(m)

    def readwrite_loop_function(self, state):
        if self.target_x < 0:
            vel_x = 0
            vel_y = 0
        else:
            dx = self.target_x - 0.5
            dy = self.target_y - 0.5
            speedx = abs(dx) * self.pantilt_speed
            speedy = abs(dy) * self.pantilt_speed
            dir_x = -1 if dx > 0 else 1
            dir_y = 1 if dy > 0 else -1
            vel_x = int(dir_x * speedx)
            vel_y = int(dir_y * speedy)
        
        return {
            5: {rcd.DXLControlTable.GoalVelocity: vel_x}, 
            6: {rcd.DXLControlTable.GoalVelocity: vel_y},
        }
