# roboflex.metrics_central

Metrics Central has two parts:

1. The Profiler node. This node is intended to act as the root node of a computation graph. See metrics_central/examples/profile_graph.cpp, and examples/camera_follow/camera_follow.py for examples of how to us it. It brings two features:

    1.1. When you call `start()` or `stop()` on this node, it will walk the computation graph and start or stop all RunnableNodes.

    1.2. When you call `start(true)`, it will first inject a new MetricsNode between every two connected nodes. It will then drive publishing of Metrics information, which can be viewed with:

2. The MetricsCentral program. This program displays Metrics information for a running computation graph. Every row is a connection, and every column is some information about that connection, including frequency of invokation, latency, Bytes/sec, etc.

    bazel run -c opt //roboflex/metrics_central:metrics_central

![](metrics_central_1.png)