from .pyblast4_archive import (
    Blast4Archive,
    decode_query_ids,
    decode_subject_ids,
    ObjectIStream,
    SerialDataFormat
)

def as_enum(enum, name):
    if isinstance(name, str):
        return getattr(enum, name)
    elif isinstance(name, int):
        return enum(name)
    else:
        return name

def _Blast4Archive_from_path(cls, f, form):
    form = as_enum(SerialDataFormat, form)
    return _Blast4Archive_from_istream(cls, ObjectIStream.open(f, form))

def _Blast4Archive_from_istream(cls, istr):
    b4 = cls()
    b4.read_from_stream(istr)
    return b4

def _Blast4Archive_from_file(cls, f, form):
    form = as_enum(SerialDataFormat, form)
    return _Blast4Archive_from_istream(
        cls,
        ObjectIStream.from_python_file_like(form, f),
    )


Blast4Archive.from_path = classmethod(_Blast4Archive_from_path)
Blast4Archive.from_file = classmethod(_Blast4Archive_from_file)

__all__ = [
    "Blast4Archive",
    "decode_query_ids",
    "decode_subject_ids",
    "SerialDataFormat"
]
