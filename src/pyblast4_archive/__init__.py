from .pyblast4_archive import (
    Blast4Archive,
    decode_internal_ids,
    ObjectIStream,
    SerialDataFormat
)

def _Blast4Archive_from_file(cls, f, form):
    if isinstance(form, str):
        form = getattr(SerialDataFormat, form)
    istr = ObjectIStream.open(f, form)
    b4 = cls()
    b4.read_from_stream(istr)
    return b4

Blast4Archive.from_file = classmethod(_Blast4Archive_from_file)

__all__ = ["Blast4Archive", "decode_internal_ids", "SerialDataFormat"]
