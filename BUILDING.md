# BUILDING roboflex

Roboflex currently uses bazel to build. We recommend installing 'bazelisk', which will install and continously update bazel.

If you have all system dependencies installed, you can build the world like this:

    bazel build -c opt //...

...however, that will build roboflex.core AND all other components. If, for instance, you aren't using the realsense driver, then you don't need to build roboflex.realsense (and in fact, you won't be able top without installing some system libraries that you just don't need).

We recommend building by component. For example:

    bazel build -c opt //core/...

Each component will have documentation describing any system dependencies, how to build, and how to run tests.
