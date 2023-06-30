# BUILDING roboflex

Roboflex currently uses bazel to build. We recommend installing 'bazelisk', which will install and continously update bazel.

If you have all system dependencies installed, you can build the world like this:

    bazel build -c opt //...

...however, that will build roboflex.core AND all other components. If, for instance, you aren't using the realsense driver, then you don't need to build roboflex.realsense (and in fact, you won't be able top without installing some system libraries that you just don't need).

We recommend building by component. For example:

    bazel build -c opt //core/...

Each component will have documentation describing any system dependencies, how to build, and how to run tests.

# RUNNING roboflex using bazel

### Bazel provides certain benefits: 

* distributed/cached artifact compilation
* interoperation with python and c++
* relatively easy syntax and 
* hermeticity (well...)

### Bazel run:

Perhaps nicest, it provides `bazel run`, which can run both c++ and python programs, and build-afresh any changed input artifacts or libraries that it depends on. We rely on this extensively.

    bazel run -c opt //core/examples/cpp:tensors_0
    bazel run -c opt //core/examples/python:tensors_0


### Bazel also has certain problems: 

* Documentation is extensive, but poorly organized. "How do I just do X" questions difficult to answer without a lot of digging. Not easy to get started.
* Hermeticity is cool and all, but it gets broken a bit every time a system library is relied on. "Building the world" just isn't feasable. 
* It doesn't deal well with distributed development; repos that want to include/link to other repos. Maybe bazel modules will solve it? It feels bolted on.
 

# The verdict

We feel that bazel is best at building monorepos. However, because we're sort of all about distributed development (I mean, look at our protocol), we're definitely interested in other build systems as well. Is cmake the answer? It seems to be almost the polar opposite of bazel; easy to answer "how do I do X", but hard to figure out how it works. Leads to spaghetti. The benefit: everything else supports it.