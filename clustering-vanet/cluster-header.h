#ifndef CLUSTER_HEADER_H
#define CLUSTER_HEADER_H

#include <map>
#include "cluster-support.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include <numeric>
#include <iostream>
namespace ns3{
/**
 * \class BeaconExHeader
 * \brief Packet header for Clustering Vanet application.
 *
 * The header is made of a 64bits sequence number followed by
 * a 64 bits time stamp and
 * a mobility information structure.
 */
class BeaconExHeader : public Header {
  public:
    BeaconExHeader ();
    virtual ~BeaconExHeader ();
    static TypeId GetTypeId (void);
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual void Print (std::ostream &os) const;
    virtual TypeId GetInstanceTypeId (void) const;
    Time GetTs (void) const;
    // void SetSeq (uint64_t seq);
    // uint64_t GetSeq (void) const;
    void SetBeaconInfo (ClusterSupport::BeaconInfo info);
    ClusterSupport::BeaconInfo GetBeaconInfo (void);

  private:
    // uint64_t m_seq;
    uint64_t m_ts;
    ClusterSupport::BeaconInfo beaconInfo;
};

/**
 * \class NeighborListExHeader
 * \brief Packet header for Clustering vanet Application
 * The header is made of a 64 bits sequence number followed by 
 * a 64 bits time stamp and
 * a map of int and mobility info structure
 */
// class NeighborListExHeader : public Header
// { 
//   public:
//     NeighborListExHeader ();
//     virtual ~NeighborListExHeader ();
//     static TypeId GetTypeId (void);
//     virtual uint64_t GetSerializedSize (void) const;
//     virtual void Serialize (Buffer::Iterator start) const;
//     virtual uint64_t Deserialize (Buffer::Iterator start);
//     virtual void Print (std::ostream &os) const;
//     virtual TypeId GetInstanceTypeId (void) const;
//     Time GetTs (void) const;
//     void SetSeq (uint64_t seq);
//     uint64_t GetSeq (void) const;

//   private:
//     uint64_t m_seq;
//     uint64_t m_ts;
//     std::map<uint64_t, ClusterSupport::BeaconInfo> neighborList;
// };

/**
 * \class FormClusterHeader
 * \brief Packet header for Clustering vanet Application
 * The header is made of a 64 bits sequence number followed by 
 * a mobility info structure
 */
class FormClusterHeader : public Header {
  public:
    FormClusterHeader ();
    virtual ~FormClusterHeader ();
    static TypeId GetTypeId (void);
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual void Print (std::ostream &os) const;
    virtual TypeId GetInstanceTypeId (void) const;
    Time GetTs (void) const;
    void SetSeq (uint64_t seq);
    uint64_t GetSeq (void) const;
    void SetClusterId (uint64_t cluster_id);
    uint64_t GetClusterId ();
    void SetNodeId (uint64_t node_id);
    uint64_t GetNodeId ();

    // void SetMobilityInfo (ClusterSupport::NodeInfo info);
    // ClusterSupport::NodeInfo GetMobilityInfo (void);
    void SetTs ();

  private:
    uint64_t m_seq;
    uint64_t m_ts;
    uint64_t clusterId;
    uint64_t nodeId;
    // ClusterSupport::NodeInfo mobilityInfo;
};

/**
 * \class DataPacketHeader
 * \brief Packet header for Clustering vanet Application
 * The header is made of a 64 bits sequence number followed by 
 * a 64 bits time stamp and
 * a data info structure
 */
class DataPacketHeader : public Header {
  public:
    DataPacketHeader ();
    virtual ~DataPacketHeader ();
    static TypeId GetTypeId (void);
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual void Print (std::ostream &os) const;
    virtual TypeId GetInstanceTypeId (void) const;
    Time GetTs (void) const;
    void SetTs ();
    void SetSeq (uint64_t seq);
    uint64_t GetSeq (void) const;
    // void SetDataInfo (ClusterSupport::DataInfo info);
    // ClusterSupport::DataInfo GetDataInfo (void);

  private:
    uint64_t m_seq;
    uint64_t m_ts;
    // ClusterSupport::DataInfo m_data;
};
} // namespace ns3
#endif