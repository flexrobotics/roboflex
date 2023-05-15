# This is a trampoline!
# It is used to load the python extension module when 
# running with bazel. Otherwise, bazel-run python programs
# would have to load roboflex.transport.zmq via 'import roboflex.transport.zmq.python'
# instead of 'import roboflex.transport.zmq', which is what we want.

try:
    from roboflex.transport.zmq.python import *
except Exception as e:
    print("Error trying to import roboflex.transport.zmq.python:", e)
    raise e
