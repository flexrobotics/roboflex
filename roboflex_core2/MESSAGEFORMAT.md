# roboflex message format

We believe that a self-describing message format is essential for truly distributed development. Hence, we use FlexBuffers (https://flatbuffers.dev/flexbuffers.html). FlexBuffers has these advantages:

1. Schema-less (self-describing). This means that a program can read any message without knowing how it was generated. It is still possible to impose a schema on messages, but this is entirely optional. Nodes can be written that don't know anything about how messages were generated, and that don't require an environment to be sourced, or to be built in a specific way.

2. Performance: FlexBuffers can map data structures (such as tensors) directly in memory, without copying. This allows 0-copy strategies, for high performance. There is a slight performance penalty compared to flatbuffers in interpreting the data structures, but we assess this to be acceptable, especially considering that the dominant time factor will almost always be the processing of whatever tensors get sent around.

3. Wide support: FlexBuffers is .h-only library, and has wide support in other programming languages.

We make a few additions:

1. Roboflex messages start with an 8-byte header: 4 bytes are an identifier "RFLX", the next four bytes are the total size of the message. After the header, all data is encoded in FlexBuffers.

2. Flexbuffer messages at root can be anything: a map, a vector, a scalar, etc. Roboflex messages, however, must all be a map at their root. This map will contain at least one key: "_meta". The value for the "_meta" key is non-typed vector, and contains meta information about the message and the sender:

        "_meta": [
            timestamp (64-bit float),
            message counter (64-bit unsigned int),
            the guid of the node that sent this message (8-byte string),
            the name of the node that sent this message (32-byte string),
            the name of the module containing the message (string),
            the name of the message (string),
        ]

3. Tensors, by convention, will be maps:

        "mytensor": {
            "shape": Vector or TypedVector of uint64,
            "dtype": Int, <- maps to numpy datatypes
            "data": Blob,
        }

See [serialization/flex_eigen.h](serialization/flex_eigen.h) for code that can serialize and deserialize matrices from the popular 'eigen' library, and [serialization/flex_xtensor.h](serialization/flex_xtensor.h) for code that can serialize and deserialize tensors from the popular 'xtensor' library.


