#include "cluster-header.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("Cluster Header");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (MyHeader);

MyHeader::MyHeader () : m_ts (Simulator::Now ().GetTimeStep ())
{
  NS_LOG_FUNCTION (this);
}
MyHeader:: ~MyHeader(){
  NS_LOG_FUNCTION (this);
}

TypeId
MyHeader::GetTypeId (void){
  static TypeId tid = TypeId ("ns3::MyHeader").SetParent<Header>();
  return tid;
}

uint32_t
MyHeader::GetSerializedSize (void) const{
  NS_LOG_FUNCTION (this);
  return sizeof (uint64_t) + sizeof (uint64_t) + sizeof (uint64_t);
}

void 
MyHeader::Serialize (Buffer::Iterator start) const{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU64 (m_seq);
  i.WriteHtonU64 (m_ts);
  i.WriteHtonU64 (m_packetId);
   
}
uint32_t
MyHeader::Deserialize (Buffer::Iterator start){
  NS_LOG_INFO (this << &start);
  Buffer::Iterator i = start;
  m_seq = i.ReadNtohU64();
  m_ts = i.ReadNtohU64 ();
  m_packetId = i.ReadNtohU64 ();
  return GetSerializedSize ();
}
void
MyHeader::Print (std::ostream &os) const{
  NS_LOG_FUNCTION (this << &os);
  os << "(time=" << TimeStep (m_ts).GetSeconds ()<< "sequence: "<<m_seq<< "Packet Id: "<<m_packetId<<")";
}

TypeId
MyHeader::GetInstanceTypeId (void) const{
  return GetTypeId ();
}

Time 
MyHeader::GetTs() const {
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

void 
MyHeader::SetSeq(uint64_t seq){
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}

uint64_t 
MyHeader:: GetSeq() const {
  return m_seq;
}

void MyHeader::SetPacketId (uint64_t id){
  m_packetId = id;
}
uint64_t MyHeader::GetPacketId () const{
  return m_packetId;
}
}
