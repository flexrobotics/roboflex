import time
import imu_bno055 as imu 
import roboflex.core as rfc

class IMUPrinter(rfc.Node):
    def receive(self, m):
        print(m.timestamp, " Q:", m['q'])

node = imu.IMU_BNO055_Node()

node > IMUPrinter()

node.start()
time.sleep(2)
node.stop()

print("DONE")