#include <iostream>
#include <functional>
#include <string>
#include <istream>
#include <memory>
#include <strstream>
#include <boost/python.hpp>
#include <corelib/ncbistd.hpp>
#include <serial/serial.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <objects/blast/Blast4_archive.hpp>
#include <objects/blast/Blast4_request.hpp>
#include <objects/blast/Blast4_request_body.hpp>
#include <objects/blast/Blast4_request_body.hpp>
#include <objects/blast/Blast4_queue_search_reques.hpp>
#include <objects/blast/Blast4_queries.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <corelib/ncbiobj.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/general/User_object.hpp>
#include <objects/blast/Blast4_subject.hpp>
#include <objtools/blast/seqdb_reader/seqdb.hpp>
#include <objects/blast/Blas_get_searc_resul_reply.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include "boost/python/class.hpp"
#include "boost/python/ssize_t.hpp"
#include "python_streambuf.h"

using namespace boost::python;

std::size_t boost_adaptbx::python::streambuf::default_buffer_size = 1024;

// Disambiguated overloaded function ncbi::CObjectIStream::Open.
static ncbi::CObjectIStream *(*CObjectIStreamOpen2)(
    const std::string &, ncbi::ESerialDataFormat) = &ncbi::CObjectIStream::Open;

// Disambiguated overloaded function ncbi::CObjectIStream::CreateFromBuffer.
static ncbi::CObjectIStream *(*CreateFromBuffer3)(
    ncbi::ESerialDataFormat, const char *buffer,
    size_t size) = &ncbi::CObjectIStream::CreateFromBuffer;

/* Wraps a CSerialObject to provide wrappers for the >> and << operators with
   CObjectIStream and CObjectOStream, facilitating use with boost::python. */
template <typename T>
class WrappedSerialObject: virtual public T {
public:
  WrappedSerialObject() : T() {};

  /* Reads data into the CSerialObject from a given CObjectIStream. */
  void read_from_stream(ncbi::CObjectIStream& is) {
    is >> *((ncbi::CSerialObject*)this);
  }

  /* Writes data from the CSerialObject into a given CObjectOStream. */
  void write_to_stream(ncbi::CObjectOStream& os) {
    os << *((ncbi::CSerialObject*)this);
  }
};

/* Wraps a std::ios to allow construction from a shared_ptr to the
   streambuf. When constructed with a shared_ptr, the wrapped ios stores the
   shared_ptr, keeping the streambuf alive. */
template<typename T>
class SmartStream: public T {
public:
  /* Construct a ios from a std::shared_ptr to a std::streambuf. The
     std::streambuf shall remain alive at least until this object is
     destroyed. */
  SmartStream(std::shared_ptr<std::streambuf> buf) : T(buf.get()) {
    buffer_ptr = buf;
  }
private:
  // Keeps the used streambuf alive until this object is destroyed, at least.
  std::shared_ptr<std::streambuf> buffer_ptr;
};

/* Wraps a CObjectIStream or CObjectOStream to provide a from_python_file_like
   static method allowing construction from Python file-like objects and a
   format.

   The template parameters are the underlying std::ios type, which should be a
   subclass of SmartStream so that the stream can own its streambuf, and the
   CObjectStream type to be wrapped.
*/
template <typename SmartStreamT, typename ObjectStreamT>
class WrappedObjectStream {
public:
  /* Create a stream with the given format from the given Python file-like
     object. */
  static ObjectStreamT* from_python_file_like(
    ncbi::ESerialDataFormat format,
    boost::python::object& file_like) {
    std::shared_ptr<boost_adaptbx::python::streambuf> sbp(
      new boost_adaptbx::python::streambuf(file_like, 1024)
    );
    SmartStreamT* is = new SmartStreamT(sbp);
    /* The ObjectStreamT takes ownership of the SmartStreamT instance, which, in
       turn, owns the streambuf created from the file_like object via a
       shared_ptr. */
    return ObjectStreamT::Open(format, *is, ENcbiOwnership::eTakeOwnership);
  }
};

/* Wraps CObjectIStream to provide methods for constructing instances from
   Python file-like objects (see WrappedObjectStream) or bytes. */
class WrappedObjectIStream: public ncbi::CObjectIStream,
			    public WrappedObjectStream<
			      SmartStream<std::istream>,
			      ncbi::CObjectIStream
			    > {
public:
  /* Construct an CObjectIStream with the given format from a Python bytes
     object containing the input data. */
  static CObjectIStream* from_bytes(ncbi::ESerialDataFormat format,
				    PyObject* obj) {
    char* buffer;
    Py_ssize_t length;
    int res = PyBytes_AsStringAndSize(obj, &buffer, &length);
    if (res < -1) {
      return NULL;    
    }
    return ncbi::CObjectIStream::CreateFromBuffer(format, buffer, length);
  }
};

/* Wraps CObjectIStream to provide a method for constructing instances from
   Python file-like objects (see WrappedObjectStream) and disambiguate an
   overload of CObjectOStream::Open for use with boost::python. */
class WrappedObjectOStream: public ncbi::CObjectOStream,
			    public WrappedObjectStream<
			      SmartStream<std::ostream>,
			      ncbi::CObjectOStream
			    > {
public:
  // Disambiguates CObjectOStream::Open.
  static CObjectOStream* Open2(const std::string& file_name,
			       ncbi::ESerialDataFormat format) {
    ncbi::CObjectOStream* str = WrappedObjectOStream::Open(file_name, format);
    return str;
  }
};


/* Maps BLAST internal query sequence IDs to corresponding FASTA IDs for a BLAST
   archive. */
boost::python::dict decode_one_query_ids(ncbi::objects::CBlast4_archive& b4) {
  boost::python::dict dct;
  auto seq_set = b4.GetRequest().GetBody().GetQueue_search().GetQueries()
    .GetBioseq_set().GetSeq_set();
  for (auto c: seq_set) {
    auto d = c.GetPointer();
    auto seqp = &d->GetSeq();
    auto local_id = seqp->GetLocalId()->GetSeqIdString();
    std::string title;
    for (auto k: seqp->GetDescr().Get()) {
      auto l = k.GetPointer();
      if (l->IsTitle()) {
	title = l->GetTitle();
      }
    }
    dct[local_id] = title;
  }
  return dct;
}

/* Use a function to decode all IDs in a list of CBlast4_archive objects.

   The first argument should be a function that takes a CBlast4_archive and
   returns a Python dict mapping internal IDs to their decoded FASTA IDs.

   The second argument should be the CBlast4_archive objects on which to apply
   the function.
 */
boost::python::dict decode_all(
  std::function<
    boost::python::dict(
      ncbi::objects::CBlast4_archive&
    )
  > decode_one,
  boost::python::list& archives
) {
  boost::python::dict dct;
  for (boost::python::ssize_t i = 0; i < boost::python::len(archives); i+=1) {
    ncbi::objects::CBlast4_archive& b4 =
      boost::python::extract<ncbi::objects::CBlast4_archive&>(archives[i]);
    dct |= decode_one(b4);
  }
  return dct;
}

/* Maps BLAST internal subject sequence IDs to corresponding FASTA IDs for a
   BLAST archive. */
boost::python::dict decode_one_subject_ids(ncbi::objects::CBlast4_archive& b4) {
  boost::python::dict dct;
  auto& subjects = b4.GetRequest().GetBody().GetQueue_search().GetSubject();
  if (subjects.IsSequences()) {
    auto seqs = subjects.GetSequences();
    for (auto c : seqs) {
      auto seqp = c.GetPointer();
      auto local_id = seqp->GetLocalId()->GetSeqIdString();
      std::string title;
      for (auto k : seqp->GetDescr().Get()) {
        auto l = k.GetPointer();
        if (l->IsTitle()) {
          title = l->GetTitle();
        }
      }
      dct[local_id] = title;
    }
  }
  return dct;
}

/* Maps BLAST database internal sequence OIDs to corresponding FASTA IDs for a
   BLAST archive and corresponding BLAST sequence database. */
boost::python::dict decode_one_database_oids(ncbi::objects::CBlast4_archive& b4,
					     ncbi::CSeqDB& seq_db) {
  boost::python::dict dct;
  for (auto al: b4.GetResults().GetAlignments().Get()) {
    auto& segs = al->GetSegs();
        if (!segs.IsDenseg()) {
      continue;
    }
    auto& denseg = segs.GetDenseg();
    for (auto id : denseg.GetIds()) {
      if (!id->IsGeneral()) {
        continue;
      }
      auto& general_id = id->GetGeneral();
      if (!general_id.CanGetDb()) {
	continue;
      }
      auto db = general_id.GetDb();
      if ((db != "BL_ORD_ID") || !general_id.CanGetTag()) {
	continue;
      }
      auto& tag = general_id.GetTag();
      if (!tag.IsId()) {
	continue;
      }
      int ord_id = tag.GetId();
      std::string title;
      
      const auto& bioseq = seq_db.GetBioseq(ord_id);
      for (auto k: bioseq->GetDescr().Get()) {
        auto l = k.GetPointer();
        if (l->IsTitle()) {
          title = l->GetTitle();
        }
      }
      dct[ord_id] = title;
    }
  }
  return dct;
}

BOOST_PYTHON_MODULE(pyblast4_archive) {
  class_<ncbi::CObjectIStream, boost::noncopyable>("_ObjectIStream", no_init)
    .def("open", CObjectIStreamOpen2,
	 return_value_policy<manage_new_object>(),
	 R"END(
Open an input stream from a file path and the file's data format.

Parameters:
    file_name (str): Path to file to open.
    format:          ESerialDataFormat representing format of file to open.

Returns:
    An input stream with the specified file opened for reading in given format.
)END")
    .staticmethod("open")
    .def("_from_buffer", CreateFromBuffer3,
	 return_value_policy<manage_new_object>())
    .staticmethod("_from_buffer")
    .def("end_of_data", &ncbi::CObjectIStream::EndOfData,
	 "Check whether there is still data left to read from the stream.");

  class_<WrappedObjectIStream, boost::noncopyable,
	 bases<ncbi::CObjectIStream>>("ObjectIStream", R"END(
Input stream for reading NCBI Toolkit objects.

Unlike a Python file-like object, an ObjectIStream has some notion of the file
format used by the stream, but an ObjectIStream can be created from a Python
file-like object using the from_python_file_like staticmethod.
)END", no_init)
    .def("from_python_file_like", &WrappedObjectIStream::from_python_file_like,
	 return_value_policy<manage_new_object>(), R"END(
Create an input stream from a format and a Python file-like object.

Parameters:
    format:    ESerialDataFormat representing format of file-like object.
    file_like: Readable file-like object from which to create input stream.

Returns:
    An input stream for the file-like object using the given format.
)END")
    .staticmethod("from_python_file_like")
    .def("from_bytes", &WrappedObjectIStream::from_bytes,
	 return_internal_reference<2, return_value_policy<manage_new_object>>(),
	 R"END(
Create an input stream from a format and bytes.

Parameters:
    format:       ESerialDataFormat representing format of bytes.
    data (bytes): Bytes data for which to create input stream.

Returns:
    An input stream for the bytes using the given format.
)END")
    .staticmethod("from_bytes");

  class_<ncbi::CObjectOStream, boost::noncopyable>("_ObjectOStream", no_init);

  class_<WrappedObjectOStream, boost::noncopyable>("ObjectOStream", R"END(
Output stream for writing NCBI Toolkit objects.

Unlike a Python file-like object, an ObjectOStream has some notion of the file
format used by the stream, but an ObjectOStream can be created from a Python
file-like object using the from_python_file_like staticmethod.
)END", no_init)
    .def("open", &WrappedObjectOStream::Open2,
	 return_value_policy<manage_new_object>(),
	 	 R"END(
Open an output stream from a file path and the file's data format.

Parameters:
    file_name (str): Path to file to open for writing.
    format:          ESerialDataFormat representing format of file to open.

Returns:
    An output stream with the specified file opened for writing in given format.
)END")
    .staticmethod("open")
    .def("from_python_file_like", &WrappedObjectOStream::from_python_file_like,
	 return_value_policy<manage_new_object>(), R"END(
Create an output stream from a format and a Python file-like object.

Parameters:
    format:    ESerialDataFormat representing format of file-like object.
    file_like: Writeable file-like object from which to create output stream.

Returns:
    An output stream for the file-like object using the given format.
)END")
    .staticmethod("from_python_file_like");

  class_<ncbi::CSerialObject, boost::noncopyable>("SerialObject", "Base class "
						  "of serializable objects "
						  "from the NCBI C++ Toolkit.",
						  no_init);

  class_<ncbi::objects::CBlast4_archive, boost::noncopyable,
	 bases<ncbi::CSerialObject>>("_Blast4Archive", no_init);

  class_<WrappedSerialObject<ncbi::objects::CBlast4_archive>,
	 bases<ncbi::objects::CBlast4_archive>,
	 boost::noncopyable>("Blast4Archive",
			     R"END(
C++-based internal base class for BLAST archive format archives.

Instances of this class represent archives containing BLAST results.
)END")
    .def("read_from_stream",
	 &WrappedSerialObject<ncbi::objects::CBlast4_archive>::read_from_stream,
	 R"END(
Read a BLAST archive from an ObjectIStream.

Parameters:
    is_: The ObjectIStream from which to read the BLAST archive data.
)END")
    .def("write_to_stream",
	 &WrappedSerialObject<ncbi::objects::CBlast4_archive>::write_to_stream,
	 R"END(
Write a BLAST archive to an ObjectOStream.

Parameters:
    os_: The ObjectOStream to which to write the BLAST archive data.
)END");

  class_<ncbi::CSeqDB,
	 boost::noncopyable>("SeqDB",
			     "Internal C++ class representing a BLAST sequence "
			     "database (nucleotide or protein).",
			     boost::python::init<std::string,
			                         ncbi::CSeqDB::ESeqType>()
			     );

  enum_<ncbi::ESerialDataFormat>("SerialDataFormat",
				 "Serialization format of data to be streamed.")
    .value("none", ncbi::eSerial_None)
    .value("asn_text", ncbi::eSerial_AsnText)
    .value("asn_binary", ncbi::eSerial_AsnBinary)
    .value("xml", ncbi::eSerial_Xml)
    .value("json", ncbi::eSerial_Json);

  enum_<ncbi::CSeqDB::ESeqType>("_SeqDBSeqType")
    .value("protein", ncbi::CSeqDB::ESeqType::eProtein)
    .value("nucleotide", ncbi::CSeqDB::ESeqType::eNucleotide)
    .value("unknown", ncbi::CSeqDB::ESeqType::eUnknown);

  def("decode_query_ids", +[](boost::python::list& b4) {
    return decode_all(decode_one_query_ids, b4);
  }, R"END(
Get a dict mapping query sequence IDs used in archives to original FASTA IDs.

Although the BLAST archive format retains the original FASTA IDs used for query
sequences, it also uses some internal IDs assigned to sequences by BLAST, and
some output formats report these internal IDs rather than the original FASTA
IDs. This function can be used to decode the internal IDs for such output
formats.

Parameters:
    b4s (list): A list of Blast4Archives for which to decode query IDs.

Returns:
    A dict mapping query sequence IDs used in archives to original FASTA IDs.
)END");
  
  def("decode_subject_ids", +[](boost::python::list& b4) {
    return decode_all(decode_one_subject_ids, b4);
  }, R"END(
Get a dict mapping subject sequence IDs used in archives to original FASTA IDs.

When BLAST is provided a subject sequence FASTA file, the BLAST archive format
retains the original FASTA IDs used for the subject sequences but also uses some
internal IDs assigned to sequences by BLAST. Some output formats report these
internal IDs rather than the original FASTA IDs. This function can be used to
decode the internal IDs for such output formats.

When archives are the output of a search against a BLAST database, the BLAST
archive does not store the subject FASTA IDs; the BLAST database must be
consulted to retrieve them. In such cases, this function will not be able to
decode the subject IDs. To decode database IDs, use the decode_database_oids
function.

Parameters:
    b4s (list): A list of Blast4Archives for which to decode subject IDs.

Returns:
    A dict mapping subject sequence IDs used in archives to original FASTA IDs.
)END");
  
  def("decode_database_oids", +[](boost::python::list& b4, ncbi::CSeqDB& db) {
     return decode_all([&db](ncbi::objects::CBlast4_archive& a) {
       return decode_one_database_oids(a, db);
     }, b4);
  }, R"END(
Get a dict mapping database OIDs used in archives to original FASTA IDs.

When a BLAST database is used for a BLAST alignment, the resulting BLAST archive
reports subject IDs internal to the BLAST database, but the original FASTA IDs
for the subject sequences can also be retrieved from the database. This function
gets the original FASTA IDs for any subject sequence OIDs present in the
provided BLAST archives.

Parameters:
    b4s (list): A list of Blast4Archives for which to decode database OIDs.

Returns:
    A dict mapping BLAST database OIDs used in archives to original FASTA IDs.
)END");
}

