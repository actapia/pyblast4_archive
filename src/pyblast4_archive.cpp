#include <iostream>
#include <boost/python.hpp>
#include <corelib/ncbistd.hpp>
//#include <ncbi/ncbiapp.hpp>
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
#include <functional>
using namespace boost::python;

static ncbi::CObjectIStream *(*Open2)(
    const std::string &, ncbi::ESerialDataFormat) = &ncbi::CObjectIStream::Open;

template <typename T>
class WrappedSerialObject: virtual public T {
public:
  WrappedSerialObject() : T() {};

  void read_from_stream(ncbi::CObjectIStream& is) {
    is >> *((ncbi::CSerialObject*)this);
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

void read_from_stream3(ncbi::objects::CBlast4_archive& r, ncbi::CObjectIStream &is) { is >> r; }

BOOST_PYTHON_MODULE(pyblast4_archive) {
  class_<ncbi::CObjectIStream, boost::noncopyable>("ObjectIStream", no_init)
    .def("open", Open2, return_value_policy<manage_new_object>()).staticmethod("open");

  class_<ncbi::CSerialObject, boost::noncopyable>("CSerialObject", no_init);

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
