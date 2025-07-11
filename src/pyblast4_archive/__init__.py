import io
from .pyblast4_archive import (
    Blast4Archive as _Blast4Archive,
    decode_query_ids,
    decode_subject_ids,
    decode_database_oids,
    ObjectIStream,
    ObjectOStream,
    SerialDataFormat,
    SeqDB as _SeqDB,
    _SeqDBSeqType
)

def as_enum(enum, name):
    if isinstance(name, str):
        return getattr(enum, name)
    elif isinstance(name, int):
        return enum(name)
    else:
        return name

class SeqDB(_SeqDB):
    def __init__(self, path, seq_type):
        path = str(path)
        seq_type = as_enum(_SeqDBSeqType, seq_type)
        super().__init__(path, seq_type)

class Blast4Archive(_Blast4Archive):
    @classmethod
    def from_path(cls, f, form):
        form = as_enum(SerialDataFormat, form)
        return cls.from_istream(ObjectIStream.open(f, form))

    @classmethod
    def from_istream(cls, istr):
        res = []
        while not istr.end_of_data():
            b4 = cls()
            b4.read_from_stream(istr)
            res.append(b4)
        return res

    @classmethod
    def from_file(cls, f, form):
        form = as_enum(SerialDataFormat, form)
        return cls.from_istream(
            ObjectIStream.from_python_file_like(form, f),
        )
    
    @classmethod
    def from_bytes(cls, b, form):
        form = as_enum(SerialDataFormat, form)
        return cls.from_istream(
            ObjectIStream.from_bytes(form, b),
        )

    @classmethod
    def all_to_str(cls, l):
        return "".join(map(str, l))
    
    def write(self, file_like, form):
        form = as_enum(SerialDataFormat, form)
        ostr = ObjectOStream.from_python_file_like(form, file_like)
        self.write_to_stream(ostr)

    def __str__(self):
        strio = io.StringIO()
        self.write(strio, SerialDataFormat.asn_text)
        return strio.getvalue()
        

__all__ = [
    "Blast4Archive",
    "decode_query_ids",
    "decode_subject_ids",
    "decode_database_oids",
    "SerialDataFormat",
    "SeqDB",
]
