#ifndef CLUSTER_HEADER_H
#define CLUSTER_HEADER_H

#include <map>
#include "ns3/header.h"
#include "ns3/simulator.h"
#include <numeric>
#include <iostream>
namespace ns3{
/**
 * \class MyHeader
 * \brief Packet header for Clustering Vanet application.
 *
 * The header is made of a 64bits sequence number followed by
 * a 64 bits time stamp and
 * a mobility information structure.
 */
class MyHeader : public Header {
  public:
    MyHeader ();
    virtual ~MyHeader ();
    static TypeId GetTypeId (void);
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual void Print (std::ostream &os) const;
    virtual TypeId GetInstanceTypeId (void) const;
    Time GetTs (void) const;
    void SetSeq (uint64_t seq);
    uint64_t GetSeq (void) const;
    void SetPacketId (uint64_t id);
    uint64_t GetPacketId (void)const;
    // void SetBeaconInfo (ClusterSupport::BeaconInfo info);
    // ClusterSupport::BeaconInfo GetBeaconInfo (void);

  private:
    uint64_t m_seq;// source id
    uint64_t m_ts;
    uint64_t m_packetId;
    // ClusterSupport::BeaconInfo beaconInfo;
};


} // namespace ns3
#endif