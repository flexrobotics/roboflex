# RoboFlex vs ROS

_Written June 18, 2023. Some of this information may be out of date._

An obvious first question to ask about RoboFlex is "why", closely followed by "how does it differ from ROS"?

Trigger warning: strong, subjective, political statement ahead:

    Ros is not the solution to the problem. Ros IS the problem.

Robotics is already inherently very difficult. ROS adds an unacceptable level of accidental complexity. This is entirely based on my anecdotal experience supporting robotics research in an industrial lab. YMMV.

# Problems with ROS

## 1. Versioning

1.1. Ros1 or Ros2? They aren't really compatible. Ros1 is now (or will soon be) deprecated, but I assure you that most academic labs are still on Ros1. That transition will be painful. Or you trying to use Ros2, but your robot only has a full-featured driver for Ros1 (looking at you Franka Emika)? You can try to use the Ros1-2 bridge, but good luck.

1.2. Which version of Ros1 or 2? Are you still on Foxy? Already EOL. Galactic? Humble? Iron? The versions keep coming, and are they backwards compatible? Not sure. What about that Ros2 package you are using - the docs for it says it works with Foxy - will it work on a later release? Dunno.

## 2. Installation, Building, Artifacts

Too many system installs. Custom build tool (colcon). Creates too many build artifacts. Has too many ancillary build files (xml files, yaml, etc).

## 3. Operates Poorly with Python

It seems like python support always lags behind in Ros2. 

## 4. Poorly Documented.

It'll be 5 years before Ros2 has complete documentation. It will never have "good" documentation.

## 5. DDS

The transport layer for ROS2 is DDS. Which is great. And auto-discovery seems to work wonderfully well. But what if you can't/don't want to use DDS?

## 6. Statically-Typed Messages

A ros message, as saved to a ros bag, file, or read from DDS, cannot be read unless a ros environment is sourced. If there are any custom messages, then the SAME ros overlay that was used to generated the messages must also be sourced. This is a headache - it makes it impossible
to perform truly distributed generation. If messages were, from the start, fully self-describing (like JSON), then it would be possible to, say, ingest any arbitrary messages into a database, without knowing anything about how they were generated. 

## 7. Poor Ergonomics

The combination of poor documentation, confusing versioning, unclear concepts, verbosity, other problems lead to an unacceptable level of accidental complexity. Ergonomically, ROS2 is just very expensive. From what I've seen: this leads to robotics teams (even those with c++ experience) spending way too much 'wrestling with ros' over actually solving their robotics problems.

# RoboFlex is Different

RoboFlex addresses these issues from the ground up:

## 1. Fully Dynamic Messaging

RoboFlex chooses FlexBuffers as the message format. This format is fully self-describing, and allows efficient mapping of data structures directly into memory, which allows zero-copy techniques. This makes it fast. FlexBuffers is easy to create and read, which means that in order to interoperate with RoboFlex code, you can write software that doesn't use any RoboFlex code at all - just FlexBuffers! Note - there are many other protocols that can do what FlexBuffers does; messagepack, cap'nproto, etc. The choice of FlexBuffers was somewhat arbitrary.

## 2. Python from the Get-Go

In one way, python is the worst language for robotics, because of its lack of speed and the Global Interpreter Lock, which makes threading painful. But it has some strengths:

1. Everybody knows it, seemingly.
2. You can develop very quickly with it.
3. It seems to be the lingua-franca for computer vision. So if your robot has a camera, python is great.

RoboFlex is written in c++, with bindings in python. Python is can be used to configure the 'computation graph' for your robot, which can then run without touching python anymore (avoiding speed and gil issues). You _can_ write Nodes directly in python though. High-speed ones (such as in khz robot control loops) will obviously incur penalties.

## 3. Arbitrary Transport

RoboFlex can use any wire transport layer for pub-sub. Currently it supports ZMQ and MQTT. Wire transport is accomplished by creating a node that publishes to the transport, and another node that subscribes.

## 4. Easy build/install

For now, compiling and running roboflex requires bazel. Bazel, imho, is great for mono-repos, but for distributed repos, has problems. We are interested in cmake solutions. Ultimately, we are shooting for the ability to `pip install roboflex`.

## 5. Ergonomics

This is the aesthetic, subjective part. How quickly does the tool get out of your way? If you are writing robotics software, the goal is to maximize the time you spend actually solving your robotics problems, vs installing and wrestling with your middleware. The primary design goal of RoboFlex is to minimize this time.
