import time
import roboflex.core.python as rfc
#import roboflex.core as rfc

print(dir(rfc))

class MyProducerNode(rfc.RunnableNode):
    def child_thread_fn(self):
        for i in range(4):
            time.sleep(0.5)
            self.signal({"s": "Hello World!"})
 
class MyConsumerNode(rfc.Node):
    def receive(self, m):
        print("MyConsumerNode received:", '"' + m["s"] + '"', m)

p = MyProducerNode()
c = MyConsumerNode()
print("PRODUCER:", p)
print("CONSUMER:", c)
p > c
p.start()
p.stop()

print("DONE")
