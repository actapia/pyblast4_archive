from __future__ import annotations

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

from pathlib import Path

def as_enum(enum, name):
    """Interpret a value as an enum instance.

    The value provided can be a string, an integer, or an enum instance.

    If the value provided is a string, the value will be treated as the name of
    the enum instance, and the corresponding enum instance will be fetched from
    the enum type.

    If the value provided is an integer, the value will be treated as the value
    of the enum instance, and an enum instance will be constructed from the enum
    type using the provided value.

    If the value provided is an enum instance (or any other kind of object), it
    will be returned as-is.

    Parameters:
        enum: The enum type for which to get an instance.
        name: The value to be interpreted as an enum instance.

    Returns:
        The provided name, interpreted as an instance of the given enum type.
    """
    if isinstance(name, str):
        return getattr(enum, name)
    elif isinstance(name, int):
        return enum(name)
    else:
        return name

class SeqDB(_SeqDB):
    """Class representing a BLAST sequence database."""
    def __init__(self, path: str | Path, seq_type: str | int | _SeqDBSeqType):
        """Construct a SeqDB object from a path to the BLAST DB and its type.

        Parameters:
            path:     Path to the BLAST sequence database.
            seq_type: Sequence type of the database, "nucleotide" or "protein".
        """
        path = str(path)
        seq_type = as_enum(_SeqDBSeqType, seq_type)
        super().__init__(path, seq_type)

class Blast4Archive(_Blast4Archive):
    """Class representing BLAST results in the BLAST archive format.

    Currently this class supports reading and writing BLAST archive format files
    and can be used with some C++-based functions to decode internal IDs stored
    in BLAST archives.
    """
    @classmethod
    def from_path(
            cls,
            f: str | Path,
            form: str | int | SerialDataFormat
    ) -> list[Blast4Archive]:
        """Read Blast4Archives from a file in the provided format.

        Parameters:
            f:    Path to file containing serialized Blast4Archives.
            form: Format of data in file, usually "asn_text" or "asn_binary".

        Returns:
            A list of all Blast4Archives read from the file.
        """        
        form = as_enum(SerialDataFormat, form)
        return cls.from_istream(ObjectIStream.open(f, form))

    @classmethod
    def from_istream(cls, istr: ObjectIStream) -> list[Blast4Archive]:
        """Read Blast4Archives from an ObjectIStream."""
        res = []
        while not istr.end_of_data():
            b4 = cls()
            b4.read_from_stream(istr)
            res.append(b4)
        return res

    @classmethod
    def from_file(
            cls,
            f: io.BytesIO,
            form: str | int | SerialDataFormat
    ) -> list[Blast4Archive]:
        """Read Blast4Archives from a file-like object.

        Parameters:
            f:    File-like object open for binary reading.
            form: Serialization format of data to be read.

        Returns:
            A list of Blast4Archive objects read from the file-like object.
        """
        form = as_enum(SerialDataFormat, form)
        return cls.from_istream(
            ObjectIStream.from_python_file_like(form, f),
        )
    
    @classmethod
    def from_bytes(
            cls,
            b: bytes,
            form: str | int | SerialDataFormat
    ) -> list[Blast4Archive]:
        """Read Blast4Archives from bytes.

        Parameters:
            b (bytes): bytes containing serialized Blast4Archive objects.
            f:         Serialization format of data.

        Returns:
            A list of Blast4Archive objects read from the bytes.
        """
        form = as_enum(SerialDataFormat, form)
        return cls.from_istream(
            ObjectIStream.from_bytes(form, b),
        )
    
    def write(self, file_like: io.BytesIO, form: str | int | SerialDataFormat):
        """Write the Blast4Archive in the given serialization format to a file.

        Parameters:
            file_like: File-like object to which to write the Blast4Archive.
            form:      Serialization format to use when writing.
        """
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
