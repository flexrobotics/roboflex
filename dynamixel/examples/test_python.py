import math
import roboflex.core as src
import roboflex.dynamixel as srd

DynamixelID = 5

class MyDynamixelRobot:
    def __init__(self):
        self.dynamixel = srd.DynamixelGroupController(
            device_name='/dev/ttyUSB0',
            baud_rate=3000000,
        )
        self.dynamixel.set_position_control([DynamixelID])
        self.dynamixel.set_sync_write(
            {DynamixelID: [srd.DXLControlTable.GoalPosition],}
        )
        self.dynamixel.set_sync_read(
            {DynamixelID: [srd.DXLControlTable.PresentPosition, srd.DXLControlTable.PresentVelocity],}
        )
        self.dynamixel.enable_torque([DynamixelID])

    def readwrite_loop_function(self, state, command):
        #print("readwrite_loop_function got state:", state, " command:", command)
        dt = state.timestamp.t0 - self.t0
        if dt < 3.14:
            new_pos = int(self.p0 + 200 * math.sin(dt))
            command.set(5, srd.DXLControlTable.GoalPosition, new_pos)
            return True
        return False

    def make_it_move(self):
        state = self.dynamixel.read()
        self.p0 = state.values[5][srd.DXLControlTable.PresentPosition]
        self.t0 = state.timestamp.t0
        dynamixel_robot.dynamixel.run_readwrite_loop(self.readwrite_loop_function)
    
dynamixel_robot = MyDynamixelRobot()

dynamixel_robot.make_it_move()

print("DONE")
