import math
import time
import roboflex.core as rfc
import roboflex.dynamixel as rfd

DynamixelID = 5

# All these nodes do the same thing...

class MyDynamixelRobot(rfc.Node):
    def __init__(self):
        super().__init__("MyDynamixelRobot")
        self.last_state = None
        self.t0 = 0
        self.p0 = 0

    def receive(self, m):
        self.last_state = rfd.DynamixelGroupStateMessage(m).state
        if self.t0 == 0:
            self.t0 = self.last_state.timestamp.t0
            self.p0 = self.last_state.values[DynamixelID][rfd.DXLControlTable.PresentPosition]
        dt = self.last_state.timestamp.t0 - self.t0
        new_pos = int(self.p0 + 500 * math.sin(dt))
        command = rfd.DynamixelGroupCommand()
        command.set(5, rfd.DXLControlTable.GoalPosition, new_pos)
        m = rfd.DynamixelGroupCommandMessage(command)
        self.signal(m)


class MyDynamixelRobotFrequencyTrigger(rfc.FrequencyGenerator):
    def __init__(self):
        super().__init__(30.0, "MyDynamixelRobot")
        self.last_state = None
        self.t0 = 0
        self.p0 = 0

    def receive(self, m):
        if m.message_name == rfd.DynamixelGroupStateMessage.MessageName:
            self.last_state = rfd.DynamixelGroupStateMessage(m).state

    def on_trigger(self, t):
        if self.last_state:
            if self.t0 == 0:
                self.t0 = self.last_state.timestamp.t0
                self.p0 = self.last_state.values[DynamixelID][rfd.DXLControlTable.PresentPosition]
            dt = self.last_state.timestamp.t0 - self.t0
            new_pos = int(self.p0 + 500 * math.sin(dt))
            command = rfd.DynamixelGroupCommand()
            command.set(5, rfd.DXLControlTable.GoalPosition, new_pos)
            m = rfd.DynamixelGroupCommandMessage(command)
            self.signal(m)


class MyDynamixelRemoteFrequencyController(rfd.DynamixelRemoteFrequencyController):
    def __init__(self):
        super().__init__(20.0)
        self.t0 = 0

    def readwrite_loop_function(self, state):
        if self.t0 == 0:
            self.t0 = state.timestamp.t0
            self.p0 = state.values[DynamixelID][rfd.DXLControlTable.PresentPosition]
        dt = state.timestamp.t0 - self.t0
        new_pos = int(self.p0 + 500 * math.sin(dt))
        return {5: {rfd.DXLControlTable.GoalPosition: new_pos}}
    
class MyDynamixelRemoteController(rfd.DynamixelRemoteController):
    def __init__(self):
        super().__init__()
        self.t0 = 0

    def readwrite_loop_function(self, state):
        if self.t0 == 0:
            self.t0 = state.timestamp.t0
            self.p0 = state.values[DynamixelID][rfd.DXLControlTable.PresentPosition]
        dt = state.timestamp.t0 - self.t0
        new_pos = int(self.p0 + 500 * math.sin(dt))
        return {5: {rfd.DXLControlTable.GoalPosition: new_pos}}


dynamixel_node = rfd.DynamixelGroupNode(
    controller = rfd.DynamixelGroupController.PositionController(
        device_name='/dev/ttyUSB0',
        baud_rate=3000000,
        dxl_ids=[DynamixelID],
    ),
)

controller = MyDynamixelRemoteController()

dynamixel_node > controller
controller > dynamixel_node

dynamixel_node.start()
#controller.start()

time.sleep(6.35)

dynamixel_node.stop()
#controller.stop()

print("DONE")
