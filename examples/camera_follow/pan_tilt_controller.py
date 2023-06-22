import time
import roboflex.core as rcc
import roboflex.transport.zmq as rcz
import roboflex.dynamixel as rcd


class PanTiltController(rcd.DynamixelRemoteController):

    # Inherits roboflex.dynamixe.DynamixelRemoteController
    # so that it gets the readwrite_loop_function() method
    # to override. It is also a roboflex.core.Node so that
    # it can process messages, which it does, from Nodes feeding
    # into this one with 'x' and 'y' as goal position relative
    # to me. It then converts that to a velocity command to
    # feed to the dynamixel group (2 dynamixels in this case,
    # arranged in a pan-tilt configuration).
    #
    # So in sum, this is a Node that can be fed with 'x' and 'y',
    # and will manuever the pan-tilt to point at that location,
    # with the robot running in a remote thread.

    def __init__(self, pantilt_speed = 50.0):
        super().__init__("pan_tilt_controller")

        self.pantilt_speed = pantilt_speed

        # Instantiate a DynamixelGroupNode - just a node that
        # can talk to a DynamixelGroupController. Give it one 
        # pre-configured as a VelocityController.
        self.dynamixel_node = rcd.DynamixelGroupNode(
            controller = rcd.DynamixelGroupController.VelocityController(
                device_name='/dev/ttyUSB0',
                baud_rate=3000000,
                dxl_ids=[5,6],
            ),
        )

        # Circularity is fine, I'm sure...
        self > self.dynamixel_node
        self.dynamixel_node > self

        self.target_x = -1.0
        self.target_y = -1.0

    # Called on upstream node's threads.
    def receive(self, m):
        if 'x' in m.keys() and 'y' in m.keys():
            # We've received the target. Save it.
            self.target_x = m["x"]
            self.target_y = m["y"]
        else:
            super().receive(m)

    def readwrite_loop_function(self, state):
        """
        This is the function that gets called in a loop by the
        controller, remotely linked to the dynamixel group.
        Usually about 60hz.
        """
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
        
        # Return a dynamixel group command.
        return {
            5: {rcd.DXLControlTable.GoalVelocity: vel_x}, 
            6: {rcd.DXLControlTable.GoalVelocity: vel_y},
        }
