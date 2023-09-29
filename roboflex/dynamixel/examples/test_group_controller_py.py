import math
import time
import roboflex.core as src
import roboflex.dynamixel as srd

DynamixelID = 5

class MyDynamixelRobot:
    def __init__(self):

        # CONSTRUCTOR 1: full control, each dynamixel has its own  
        # operating mode and control table entries.

        # self.dynamixel = srd.DynamixelGroupController(
        #     device_name='/dev/ttyUSB0',
        #     baud_rate=3000000,
        #     dxl_ids=[DynamixelID],
        #     operating_modes=[srd.OperatingMode.PositionControl],
        #     read_control_map={DynamixelID: [srd.DXLControlTable.PresentPosition, srd.DXLControlTable.PresentVelocity],},
        #     write_control_map={DynamixelID: [srd.DXLControlTable.GoalPosition],},
        # )

        # CONSTRUCTOR 2: every dynamixel treated the same, you supply  
        # operating mode and control table entries
        # self.dynamixel = srd.DynamixelGroupController(
        #     device_name='/dev/ttyUSB0',
        #     baud_rate=3000000,
        #     dxl_ids=[DynamixelID],
        #     operating_mode=srd.OperatingMode.PositionControl,
        #     read_control_list=[srd.DXLControlTable.PresentPosition, srd.DXLControlTable.PresentVelocity],
        #     write_control_list=[srd.DXLControlTable.GoalPosition],
        # )

        # CONSTRUCTOR 3: position control only, with default control table entries
        self.dynamixel = srd.DynamixelGroupController.PositionController(
            device_name='/dev/ttyUSB0',
            baud_rate=3000000,
            dxl_ids=[DynamixelID],
        )
        print("READ SETTINGS: ", self.dynamixel.read_control_map)
        print("WRITE SETTINGS: ", self.dynamixel.write_control_map)

    # Will be called by the readwrite loop, very frequently
    def readwrite_loop_function(self, state, command):
        #print("readwrite_loop_function got state:", state, " command:", command)
        dt = state.timestamp.t0 - self.t0
        if dt < 3.14:
            new_pos = int(self.p0 + 200 * math.sin(dt))
            command.set(5, srd.DXLControlTable.GoalPosition, new_pos)
            return True
        return False

    def make_it_move(self):

        # read current position and time
        state = self.dynamixel.read()
        self.p0 = state.values[DynamixelID][srd.DXLControlTable.PresentPosition]
        self.t0 = state.timestamp.t0

        # start the readwrite loop
        dynamixel_robot.dynamixel.run_readwrite_loop(self.readwrite_loop_function)
    

dynamixel_robot = MyDynamixelRobot()
dynamixel_robot.make_it_move()

print("DONE")
