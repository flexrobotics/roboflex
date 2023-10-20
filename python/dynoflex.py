import time
import numpy as np
import collections.abc as collections
from .roboflex_core_python_ext import (
    Message,
    MessageBackingStoreVector,
)
from roboflex.flexbuffers import Builder, Loads
from roboflex.flextensors import flex_decode, build_tensor



def _serialize(fbb, v, name=None):
    if isinstance(v, (np.floating, float)):
        if name:
            fbb.Float(name, v, byte_width=8)
        else:
            fbb.Float(v, byte_width=8)
    elif isinstance(v, (np.integer, int)):
        if name:
            fbb.Int(name, v)
        else:
            fbb.Int(v)
    elif isinstance(v, str):
        if name:
            fbb.String(name, v)
        else:
            fbb.String(v)
    elif isinstance(v, bool):
        if name:
            fbb.Bool(name, v)
        else:
            fbb.Bool(v)
    elif isinstance(v, dict):
        with fbb.Map(name):
            for k in v:
                _serialize(fbb, v[k], str(k))
    elif isinstance(v, bytes) or isinstance(v, bytearray):
        if name:
            fbb.Blob(name, v)
        else:
            fbb.Blob(v)
    elif isinstance(v, collections.Sequence):
        with fbb.Vector(name):
            for val in v:
                _serialize(fbb, val)
    elif isinstance(v, np.ndarray):
        build_tensor(fbb, v, name)

def _serialize_to_data(d, message_name):
    empty_byte_array = bytes(16)
    empty_name = "                                "
    actual_data = {
        "_meta": [
            time.time(),
            -1,
            empty_byte_array,
            empty_name,
            "core",
            message_name,
        ]
    }
    actual_data.update(d)
    fbb = Builder()
    fbb._buf = bytearray(8)
    _serialize(fbb, actual_data)
    b = fbb.Finish()
    r = MessageBackingStoreVector.copy_from(bytes(b))
    return r

def _flex_decode_message(payload):
    """ Decodes a flex message into actual data """
    flex_bytes = payload.bytes
    raw_decoded_data = Loads(flex_bytes)
    return flex_decode(raw_decoded_data)


class DynoFlex(Message):

    DefaultName = "DynoFlex"

    def __init__(self, module_name, message_name, payload):
        super().__init__(module_name, message_name, payload)
        self._d = None

    @classmethod
    def from_data(cls, data={}, name=DefaultName):
        return cls("core", name, _serialize_to_data(data, name))

    @classmethod
    def from_msg(cls, other):
        return cls(other.module_name, other.message_name, other.payload)

    @property
    def d(self):
        if self._d is None:
            self._d = _flex_decode_message(self.payload)
        return self._d

    def __getitem__(self, k):
        return self.d[k]

    def keys(self):
        return self.d.keys() if hasattr(self.d, "keys") else None

    def to_string(self):
        return f"<DynoFlex name={self.message_name} primary keys:{self.d.keys()} {super().to_string()}>"
