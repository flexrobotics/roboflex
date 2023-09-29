# This is a trampoline!
# It is used to load the python extension module when 
# running with bazel. Otherwise, bazel-run python programs
# would have to load roboflex.transport.mqtt via 'import roboflex.transport.mqtt.python'
# instead of 'import roboflex.transport.mqtt', which is what we want.

try:
    from roboflex.transport.mqtt.python import *
except Exception as e:
    print("Error trying to import roboflex.transport.mqtt.python:", e)
    raise e
