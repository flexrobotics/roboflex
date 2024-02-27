import time
import numpy as np


# --- utils ---

def _dicthaskeys(d, keys):
    """Does the dict have all those keys?"""
    assert isinstance(d, dict)
    return all([v in keys for v in d])

def _dicthasonlykeys(d, keys):
    """Does the dict have only those keys?"""
    assert isinstance(d, dict)
    return len(d) == len(keys) and all([v in keys for v in d])


# a tensormap is a map of {
#   shape: list of ints i.e. (3, 4)
#   data: raw byte data
#   dtype: int
# }
# that can be serialized to and from a numpy tensor via flexbuffer
def _istensormap(v):
    return (
        isinstance(v, dict)
        and _dicthaskeys(v, ["shape", "data", "dtype"])
        and isinstance(v["shape"], list)
        and isinstance(v["data"], bytes)
        and isinstance(v["dtype"], int)
    )


# type encoding/decoding

supported_numpy_types = [
    np.int8,
    np.int16,
    np.int32,
    np.int64,
    np.uint8,
    np.uint16,
    np.uint32,
    np.uint64,
    np.intp,
    np.uintp,
    np.float32,
    np.float64,
    np.complex64,
    np.complex128,
    np.float16,
]

def _encode_dtype(t):
    return supported_numpy_types.index(t)

def _decode_dtype(i):
    return supported_numpy_types[i]


# --- serialize tensor ---

def build_tensor(
    #fbb_builder: flexbuffers.Builder, np_tensor: np.ndarray, name: str = None
    fbb_builder, np_tensor: np.ndarray, name: str = None
):

    """Use during normal flexbuffer construction when you want to add a tensor.
    i.e.:
        ...
        fbb.Float("somefloat", 12334234.454346356, 8) <8 = numbytes per float>
        build_tensor(fbb, t, "sometensor")
        fbb.String("somestring", "check out FlexMessageCallback")
        ...
    """
    t = np_tensor
    with fbb_builder.Map(name):
        fbb_builder.TypedVectorFromElements("shape", t.shape) #_shape_padded(t.shape))
        fbb_builder.Blob("data", bytes(t.data)) # the expensive part
        fbb_builder.Int("dtype", _encode_dtype(t.dtype))


# --- numpify flex-deserialized maps ---

def _numpy_tensor_from_tensor_map(m):
    assert _istensormap(m)
    shape = m["shape"]
    if len(shape) == 0:
        return None
    dtype = _decode_dtype(m["dtype"])
    buffer = m["data"]
    npv = np.frombuffer(buffer, dtype)
    r = npv.reshape(shape)
    return r


def _flex_decode_dict(d):
    return {k: flex_decode(v) for k, v in d.items()}


def _flex_decode_list(l):
    return [flex_decode(v) for v in l]


def flex_decode(x):

    """Turns the results of flexbuffers.Loads(somedata) into
    the same thing, but where we encounter a tensormap we instantiate
    a numpy tensor instead. i.e.
    (TODO) 0-copy

        numpyfied_data = flex_decode(flexbuffers.Loads(encoded_data))
    """
    if _istensormap(x):
        return _numpy_tensor_from_tensor_map(x)
    elif isinstance(x, dict):
        return _flex_decode_dict(x)
    elif isinstance(x, list):
        return _flex_decode_list(x)
    else:
        return x
