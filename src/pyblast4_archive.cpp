#include <iostream>
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
#include <string>
#include <istream>
#include <memory>
#include <strstream>
#include "python_streambuf.h"

using namespace boost::python;

std::size_t boost_adaptbx::python::streambuf::default_buffer_size = 1024;

static ncbi::CObjectIStream *(*Open2)(
    const std::string &, ncbi::ESerialDataFormat) = &ncbi::CObjectIStream::Open;

//static ncbi::CObjectIStream *(*Open3

static ncbi::CObjectIStream *(*CreateFromBuffer3)(ncbi::ESerialDataFormat, const char* buffer, size_t size) = &ncbi::CObjectIStream::CreateFromBuffer;

template <typename T>
class WrappedSerialObject: virtual public T {
public:
  WrappedSerialObject() : T() {};

  void read_from_stream(ncbi::CObjectIStream& is) {
    is >> *((ncbi::CSerialObject*)this);
  }
};

class SmartIStream: public std::istream {
public:
  SmartIStream(std::shared_ptr<std::streambuf> buf) : std::istream(buf.get()) {
    buffer_ptr = buf;
  }
private:
  std::shared_ptr<std::streambuf> buffer_ptr;
};

class WrappedObjectIStream: public ncbi::CObjectIStream {
public:
  static CObjectIStream* from_python_file_like(ncbi::ESerialDataFormat format, boost::python::object& file_like) {
    std::shared_ptr<boost_adaptbx::python::streambuf> sbp(new boost_adaptbx::python::streambuf(file_like, 1024));
    SmartIStream* is = new SmartIStream(sbp);
    return ncbi::CObjectIStream::Open(format, *is, ENcbiOwnership::eTakeOwnership);
  }
};



boost::python::dict decode_internal_ids(ncbi::objects::CBlast4_archive& b4) {
  boost::python::dict dct;
  auto seq_set = b4.GetRequest().GetBody().GetQueue_search().GetQueries().GetBioseq_set().GetSeq_set();
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

BOOST_PYTHON_MODULE(pyblast4_archive) {
  // class_<boost_adaptbx::python::streambuf>("_PythonStreambuf");

  // class_<boost_adaptbx::python::streambuf::istream>("_PythonStreambufIStream");
  
  class_<ncbi::CObjectIStream, boost::noncopyable>("_ObjectIStream", no_init)
    .def("open", Open2, return_value_policy<manage_new_object>()).staticmethod("open")
    .def("_from_buffer", CreateFromBuffer3, return_value_policy<manage_new_object>()).staticmethod("_from_buffer");

  class_<WrappedObjectIStream, boost::noncopyable, bases<ncbi::CObjectIStream>>("ObjectIStream", no_init).
    def("from_python_file_like", &WrappedObjectIStream::from_python_file_like, return_value_policy<manage_new_object>()).staticmethod("from_python_file_like");

  class_<ncbi::CSerialObject, boost::noncopyable>("SerialObject", no_init);

  class_<ncbi::objects::CBlast4_archive, boost::noncopyable, bases<ncbi::CSerialObject>>("_Blast4Archive", no_init);

  class_<WrappedSerialObject<ncbi::objects::CBlast4_archive>, bases<ncbi::objects::CBlast4_archive>, boost::noncopyable>("Blast4Archive")
    .def("read_from_stream", &WrappedSerialObject<ncbi::objects::CBlast4_archive>::read_from_stream);

  enum_<ncbi::ESerialDataFormat>("SerialDataFormat")
    .value("none", ncbi::eSerial_None)
    .value("asn_text", ncbi::eSerial_AsnText)
    .value("asn_binary", ncbi::eSerial_AsnBinary)
    .value("xml", ncbi::eSerial_Xml)
    .value("json", ncbi::eSerial_Json);

  def("decode_internal_ids", decode_internal_ids);
}
