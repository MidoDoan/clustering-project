#include "cluster-header.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "cluster-support.h"

NS_LOG_COMPONENT_DEFINE ("Cluster Header");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (BeaconExHeader);

BeaconExHeader::BeaconExHeader () : m_ts (Simulator::Now ().GetTimeStep ())
{
  NS_LOG_FUNCTION (this);
}
BeaconExHeader:: ~BeaconExHeader(){
  NS_LOG_FUNCTION (this);
}

TypeId
BeaconExHeader::GetTypeId (void){
  static TypeId tid = TypeId ("ns3::BeaconExHeader").SetParent<Header>();
  return tid;
}

uint32_t
BeaconExHeader::GetSerializedSize (void) const{
  NS_LOG_FUNCTION (this);
  return sizeof (uint64_t) + sizeof (uint64_t) + sizeof (ClusterSupport::BeaconInfo);
}

void 
BeaconExHeader::Serialize (Buffer::Iterator start) const{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  // i.WriteHtonU64 (m_seq);
  i.WriteHtonU64 (m_ts);
  
  //Write mobility structure
  // unsigned char temp[sizeof (ClusterSupport::BeaconInfo)];
  // memcpy (temp, &beaconInfo, sizeof (ClusterSupport::BeaconInfo));
  // i.Write (temp, sizeof (ClusterSupport::BeaconInfo));
  // i.WriteHtonU64 (beaconInfo.clusterId);
  i.WriteHtonU64 (beaconInfo.nodeId);
  i.WriteHtonU64 (beaconInfo.posX);
  i.WriteHtonU64 (beaconInfo.posY);
  i.WriteHtonU64 (beaconInfo.posZ);
  i.WriteHtonU64 (beaconInfo.velX);
  i.WriteHtonU64 (beaconInfo.velY);
  i.WriteHtonU64 (beaconInfo.velZ);
  // i.WriteHtonU64 (beaconInfo.degree);
}
uint32_t
BeaconExHeader::Deserialize (Buffer::Iterator start){
  NS_LOG_INFO (this << &start);
  Buffer::Iterator i = start;
  // m_seq = i.ReadNtohU64();
  m_ts = i.ReadNtohU64 ();
  // beaconInfo.clusterId = i.ReadNtohU64 ();
  beaconInfo.nodeId = i.ReadNtohU64 ();
  beaconInfo.posX = i.ReadNtohU64 ();
  beaconInfo.posY = i.ReadNtohU64 ();
  beaconInfo.posZ = i.ReadNtohU64 ();
  beaconInfo.velX = i.ReadNtohU64 ();
  beaconInfo.velY = i.ReadNtohU64 ();
  beaconInfo.velZ = i.ReadNtohU64 ();
  // beaconInfo.degree = static_cast<ClusterSupport::NodeDegree>(i.ReadNtohU64 ());
  // unsigned char temp[sizeof (ClusterSupport::BeaconInfo)];
  // i.Read (temp, sizeof (ClusterSupport::BeaconInfo));
  // memcpy (&beaconInfo, &temp, sizeof (ClusterSupport::BeaconInfo));
  return GetSerializedSize ();
}
void
BeaconExHeader::Print (std::ostream &os) const{
  NS_LOG_FUNCTION (this << &os);
  os << "(time=" << TimeStep (m_ts).GetSeconds ()<< "Node ID=" << beaconInfo.nodeId
     << "Position="
     << "{" << beaconInfo.posX << "," << beaconInfo.posY << "," << beaconInfo.posZ << "}"
     << "Velocity="
     << "{" << beaconInfo.velX << "," << beaconInfo.velY << "," << beaconInfo.velZ << "}" 
     << ")";
}

TypeId
BeaconExHeader::GetInstanceTypeId (void) const{
  return GetTypeId ();
}

Time 
BeaconExHeader::GetTs() const {
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

// void 
// BeaconExHeader::SetSeq(uint64_t seq){
//   NS_LOG_FUNCTION (this << seq);
//   m_seq = seq;
// }

// uint64_t 
// BeaconExHeader:: GetSeq() const {
//   return m_seq;
// }

void 
BeaconExHeader::SetBeaconInfo (ClusterSupport::BeaconInfo info){
  beaconInfo = info;
}

ClusterSupport::BeaconInfo 
BeaconExHeader::GetBeaconInfo (void) {
  return beaconInfo;
}
////////////////////////////////////////////////////////////////////////////////////////////////
// NS_OBJECT_ENSURE_REGISTERED (NeighborListExHeader);

// NeighborListExHeader::NeighborListExHeader () : m_ts (Simulator::Now ().GetTimeStep ()), m_seq (0)
// {
//   NS_LOG_FUNCTION (this);
// }
// NeighborListExHeader::~NeighborListExHeader ()
// {
//   NS_LOG_FUNCTION (this);
// }

// TypeId
// NeighborListExHeader::GetTypeId (void)
// {
//   static TypeId tid = TypeId ("ns3::NeighborListExHeader")
//                           .SetParent<Header> ()
//                           .AddConstructor<NeighborListExHeader> ();
//   return tid;
// }

// uint64_t
// NeighborListExHeader::GetSerializedSize (void) const
// {
//   NS_LOG_FUNCTION (this);
//   return sizeof (uint64_t) + sizeof (uint64_t) + sizeof (std::map<uint64_t, ClusterSupport::BeaconInfo>);
// }

// void
// NeighborListExHeader::Serialize (Buffer::Iterator start) const
// {
//   NS_LOG_FUNCTION (this << &start);
//   Buffer::Iterator i = start;
//   i.WriteHtonU64 (m_seq);
//   i.WriteHtonU64 (m_ts);

//   //Write mobility structure
//   // unsigned char temp[sizeof (ClusterSupport::BeaconInfo)];
//   // memcpy (temp, &beaconInfo, sizeof (ClusterSupport::BeaconInfo));
//   // i.Write (temp, sizeof (ClusterSupport::BeaconInfo));
//   // i.WriteHtonU64 (mobilityInfo.clusterId); thieu serialize map
  
// }
// uint64_t
// NeighborListExHeader::Deserialize (Buffer::Iterator start)
// {
//   NS_LOG_INFO (this << &start);
//   Buffer::Iterator i = start;
//   m_seq = i.ReadNtohU64 ();
//   m_ts = i.ReadNtohU64 ();
//   mobilityInfo.clusterId = i.ReadNtohU64 ();
//   mobilityInfo.nodeId = i.ReadNtohU64 ();
//   mobilityInfo.posX = i.ReadNtohU64 ();
//   mobilityInfo.posY = i.ReadNtohU64 ();
//   mobilityInfo.posZ = i.ReadNtohU64 ();
//   mobilityInfo.velX = i.ReadNtohU64 ();
//   mobilityInfo.velY = i.ReadNtohU64 ();
//   mobilityInfo.velZ = i.ReadNtohU64 ();
//   mobilityInfo.degree = static_cast<ClusterSupport::NodeDegree> (i.ReadNtohU64 ());
//   // unsigned char temp[sizeof (ClusterSupport::BeaconInfo)];
//   // i.Read (temp, sizeof (ClusterSupport::BeaconInfo));
//   // memcpy (&beaconInfo, &temp, sizeof (ClusterSupport::BeaconInfo));
//   return GetSerializedSize ();
// }
// void
// NeighborListExHeader::Print (std::ostream &os) const
// {
//   NS_LOG_FUNCTION (this << &os);
//   os << "(seq=" << m_seq << "time=" << TimeStep (m_ts).GetSeconds ()
//      << "ClusterId=" << mobilityInfo.clusterId << "Node ID=" << mobilityInfo.nodeId << "Position="
//      << "{" << mobilityInfo.posX << "," << mobilityInfo.posY << "," << mobilityInfo.posZ << "}"
//      << "Velocity="
//      << "{" << mobilityInfo.velX << "," << mobilityInfo.velY << "," << mobilityInfo.velZ << "}"
//      << "Degree=" << mobilityInfo.degree << ")";
// }

// TypeId
// FormClusterHeader::GetInstanceTypeId (void) const
// {
//   return GetTypeId ();
// }

// Time
// FormClusterHeader::GetTs () const
// {
//   NS_LOG_FUNCTION (this);
//   return TimeStep (m_ts);
// }

// void
// FormClusterHeader::SetSeq (uint64_t seq)
// {
//   NS_LOG_FUNCTION (this << seq);
//   m_seq = seq;
// }

// uint64_t
// FormClusterHeader::GetSeq () const
// {
//   return m_seq;
// }

////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (FormClusterHeader);

FormClusterHeader::FormClusterHeader () : m_seq (0), m_ts (0)
{
  NS_LOG_FUNCTION (this);
}
FormClusterHeader::~FormClusterHeader ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
FormClusterHeader::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::FormClusterHeader").SetParent<Header> ();
  return tid;
}

uint32_t
FormClusterHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return sizeof (uint64_t) + sizeof (uint64_t) + sizeof (ClusterSupport::NodeInfo);
}

void
FormClusterHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU64 (m_seq);
  i.WriteHtonU64 (m_ts);

  //Write mobility structure
  // unsigned char temp[sizeof (ClusterSupport::BeaconInfo)];
  // memcpy (temp, &beaconInfo, sizeof (ClusterSupport::BeaconInfo));
  // i.Write (temp, sizeof (ClusterSupport::BeaconInfo));
  // i.WriteHtonU64 (mobilityInfo.clusterId);
  // i.WriteHtonU64 (mobilityInfo.nodeId);
  i.WriteHtonU64 (nodeId);
  i.WriteHtonU64 (clusterId);
  // i.WriteHtonU64 (mobilityInfo.posX);
  // i.WriteHtonU64 (mobilityInfo.posY);
  // i.WriteHtonU64 (mobilityInfo.posZ);
  // i.WriteHtonU64 (mobilityInfo.velX);
  // i.WriteHtonU64 (mobilityInfo.velY);
  // i.WriteHtonU64 (mobilityInfo.velZ);
  // i.WriteHtonU64 (mobilityInfo.degree);
}
uint32_t
FormClusterHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_INFO (this << &start);
  Buffer::Iterator i = start;
  m_seq = i.ReadNtohU64 ();
  m_ts = i.ReadNtohU64 ();
  nodeId = i.ReadNtohU64 ();
  clusterId = i.ReadNtohU64 ();
  // mobilityInfo.clusterId = i.ReadNtohU64 ();
  // mobilityInfo.nodeId = i.ReadNtohU64 ();
  // mobilityInfo.posX = i.ReadNtohU64 ();
  // mobilityInfo.posY = i.ReadNtohU64 ();
  // mobilityInfo.posZ = i.ReadNtohU64 ();
  // mobilityInfo.velX = i.ReadNtohU64 ();
  // mobilityInfo.velY = i.ReadNtohU64 ();
  // mobilityInfo.velZ = i.ReadNtohU64 ();
  // mobilityInfo.degree = static_cast<ClusterSupport::NodeDegree> (i.ReadNtohU64 ());
  // unsigned char temp[sizeof (ClusterSupport::BeaconInfo)];
  // i.Read (temp, sizeof (ClusterSupport::BeaconInfo));
  // memcpy (&beaconInfo, &temp, sizeof (ClusterSupport::BeaconInfo));
  return GetSerializedSize ();
}
void
FormClusterHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(seq=" << m_seq << "time=" << TimeStep (m_ts).GetSeconds () << "Node ID=" << nodeId
     << "Cluster ID= " << clusterId;
  // << "{" << mobilityInfo.posX << "," << mobilityInfo.posY << "," << mobilityInfo.posZ << "}"
  // << "Velocity="
  // << "{" << mobilityInfo.velX << "," << mobilityInfo.velY << "," << mobilityInfo.velZ << "}"
  // << ")";
}

TypeId
FormClusterHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

Time
FormClusterHeader::GetTs () const
{
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

void
FormClusterHeader::SetSeq (uint64_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}

uint64_t
FormClusterHeader::GetSeq () const
{
  return m_seq;
}

// void
// FormClusterHeader::SetMobilityInfo (ClusterSupport::NodeInfo info)
// {
//   mobilityInfo = info;
// }

// ClusterSupport::NodeInfo
// FormClusterHeader::GetMobilityInfo (void)
// {
//   return mobilityInfo;
// }

void
FormClusterHeader::SetTs(){
  m_ts = Simulator::Now().GetTimeStep();
}

void FormClusterHeader::SetClusterId (uint64_t cluster_id){
  clusterId = cluster_id;
}
uint64_t
FormClusterHeader::GetClusterId ()
{
  return clusterId;
}
void
FormClusterHeader::
SetNodeId (uint64_t node_id)
{
  nodeId = node_id;
}
uint64_t
FormClusterHeader::GetNodeId ()
{
  return nodeId;
}
////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (DataPacketHeader);

DataPacketHeader::DataPacketHeader () : m_seq (0)
{
  NS_LOG_FUNCTION (this);
}
DataPacketHeader::~DataPacketHeader ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
DataPacketHeader::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::DataPacketHeader").SetParent<Header> ();
  return tid;
}

uint32_t
DataPacketHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return sizeof (uint64_t) + sizeof (uint64_t) + sizeof (uint64_t);
}

void
DataPacketHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU64 (m_seq);
  i.WriteHtonU64 (m_ts);
  i.WriteHtonU64 (m_packetId);

  //Write mobility structure
  // unsigned char temp[sizeof (ClusterSupport::BeaconInfo)];
  // memcpy (temp, &beaconInfo, sizeof (ClusterSupport::BeaconInfo));
  // i.Write (temp, sizeof (ClusterSupport::BeaconInfo));
  // i.WriteHtonU64 (m_data);
}
uint32_t
DataPacketHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_INFO (this << &start);
  Buffer::Iterator i = start;
  m_seq = i.ReadNtohU64 ();
  m_ts = i.ReadNtohU64 ();
  m_packetId = i.ReadNtohU64 ();
  // m_data = static_cast<ClusterSupport::DataInfo> (i.ReadNtohU64 ());
  // unsigned char temp[sizeof (ClusterSupport::BeaconInfo)];
  // i.Read (temp, sizeof (ClusterSupport::BeaconInfo));
  // memcpy (&beaconInfo, &temp, sizeof (ClusterSupport::BeaconInfo));
  return GetSerializedSize ();
}
void
DataPacketHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(seq=" << m_seq << "time=" << TimeStep (m_ts).GetSeconds () ;
}

TypeId
DataPacketHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

Time
DataPacketHeader::GetTs () const
{
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

void
DataPacketHeader::SetSeq (uint64_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}

uint64_t
DataPacketHeader::GetSeq () const
{
  return m_seq;
}

// void
// DataPacketHeader::SetDataInfo (ClusterSupport::DataInfo info)
// {
//   m_data = info;
// }

// ClusterSupport::DataInfo
// DataPacketHeader::GetDataInfo (void)
// {
//   return m_data;
// }
void
DataPacketHeader::SetTs ()
{
  m_ts = Simulator::Now ().GetTimeStep ();
}

void
DataPacketHeader::SetPacketId (uint64_t id)
{
  m_packetId = id;
}
uint64_t
DataPacketHeader::GetPacketId () const
{
  return m_packetId;
}
}
