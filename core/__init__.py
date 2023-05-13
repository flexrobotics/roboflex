# This is a trampoline!
# It is used to load the python extension module when 
# running with bazel. Otherwise, bazel-run python programs
# would have to load roboflex.core via 'import roboflex.core.python'
# instead of 'import roboflex.core', which is what we want.

try:
    from roboflex.core.python import *
except Exception as e:
    print("Error trying to import oboflex.core.python:", e)
    raise e