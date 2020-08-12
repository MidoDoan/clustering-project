/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Junling Bu <linlinjavaer@gmail.com>
 */
#include "ns3/command-line.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/seq-ts-header.h"
#include "ns3/wave-net-device.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/wave-helper.h"
#include <string>
#include <numeric>
#include "conn-matrix.h"
#include "cluster-header.h"
#include "cluster-pos.h"
#include "ns3/log.h"

#define REBRCT_NUM 3
#define rsuId 2
#define Communication_range 50 // recheck it pls
#define Twaitmax 0.5
#define BeaconExTime 0.3
#define ClusterFormationTime 0.8
#define DataExchangeTime 5
#define NODE_NUM 50
#define LANE_NUM 3
#define LANE_WIDTH 2
#define PACKET_NUM 1
#define SENDER_NUM 2
using namespace ns3;
/**
 * This simulation is to show the routing service of WaveNetDevice described in IEEE 09.4.
 *
 * note: although txPowerLevel is supported now, the "TxPowerLevels"
 * attribute of YansWifiPhy is 1 which means phy devices only support 1
 * levels. Thus, if users want to control txPowerLevel, they should set
 * these attributes of YansWifiPhy by themselves..
 */
class WaveNetDeviceExample
{
public:
  /// Send WSMP example function
  void SendPacketExample (void);

  /// Send IP example function
  void SendIpExample (void);

  /// Send WSA example
  void SendWsaExample (void);

private:
  /**
   * Send one WSMP packet function
   * \param channel the channel to use
   * \param seq the sequence
   */
  void SendOneWsmpPacket (uint64_t channel, uint64_t seq, uint64_t sender_id);
  /**
   * Send IP packet function
   * \param seq the sequence
   * \param ipv6 true if IPV6
   */
  void SendIpPacket (uint64_t seq, bool ipv6);
  /**
   * Receive function
   * \param dev the device
   * \param pkt the packet
   * \param mode the mode
   * \param sender the sender address
   * \returns true if successful
   */
  bool Receive (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender);
  /**
   * Receive VSA function
   * \param pkt the packet
   * \param address the address
   * \returns true if successful
   */
  bool ReceiveVsa (Ptr<const Packet> pkt, const Address &address, uint64_t, uint64_t);
  /// Create WAVE nodes function
  void CreateWaveNodes (void);
  void CreateWaveNodeCustomMobility (std::string file_path);
  std::string MactoString (Address const &sender);
  uint64_t MactoInt (std::string const &s);
  NodeContainer nodes; ///< the nodes
  NetDeviceContainer devices; ///< the devices
  Ptr<WaveNetDevice> device;

  std::vector<uint64_t> rebroadcast_check;
  // void InitiateRebroadcastMatrix (uint64_t node_num);
  // void ReBroadcastPacket (uint64_t channel, uint64_t seq, uint64_t sender_id, Ptr<Packet> pkt);
  void SendPacket (uint64_t channel, uint64_t sender_id, uint64_t process, uint64_t receiver_id,
                   uint64_t packet_id);
  void SendFormClusterMsg (uint64_t channel, uint64_t sender_id);
  bool ReceiveFormClusterMsg ();

  ConnectivityMatrix conn_mat;
  ConnectivityMatrix delay_mat;
  double CalculateWaitingTime (uint32_t node_id, uint32_t node_num, double distance);
  double CalculateDistance (uint64_t node1, uint64_t node2);
  std::vector<double> TwaitArr;
  void SendFormationMsg (uint64_t sender_id, uint64_t channel);
  bool ReceiveFormationMsg (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode,
                            const Address &sender);
  std::vector<ClusterSupport::NodeInfo> nodeInfoList;
  void ScheduleSendPacket (Time time, uint64_t channel, uint64_t sender_id);
  void ScheduleSendPacket1 (Time time, uint64_t channel, uint64_t sender_id);
  void InitiateNodeInfo ();
  void UpdateAllNodeInfo ();
  void UpdateNodeInfo (uint64_t node_id, ClusterSupport::NodeDegree degree, uint64_t cluster_id,
                       double posX, double posY, double posZ, double velX, double velY,
                       double velZ);
  void PrintNodeListInfo ();
  // void SendFormationMsg (uint64_t channel, uint64_t sender_id);
  void SetTwaitArr ();
  void SendFormation ();
  void ReBroadcastPacket (uint32_t channel, uint64_t sender_id, Ptr<Packet> pkt);
  uint32_t rebroadcast_count = 0;
  std::vector<uint64_t> packetSrc;
  // float delay_arr[getDeviceNum()];
};
// void
// WaveNetDeviceExample::ReBroadcastDataPacket (uint64_t channel, uint64_t seq, uint64_t sender_id,
//                                          Ptr<Packet> pkt)
// {
//   Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
//   // const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
//   Mac48Address bssWildcard = Mac48Address::GetBroadcast ();
//   // const TxInfo txInfo = TxInfo (channel);
//   sender->Send (pkt, bssWildcard,0);
// }
/*My Function*/
void
WaveNetDeviceExample::CreateWaveNodeCustomMobility (std::string file_path)
{
  Position posInfo = Position (file_path);
  nodes = posInfo.GetContainer (LANE_NUM, LANE_WIDTH, NODE_NUM);

  std::cout << "The number of nodes is:" << nodes.GetN () << std::endl;

  for (uint64_t i = 0; i != nodes.GetN (); ++i)
    {
      Ptr<ConstantVelocityMobilityModel> mob =
          nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ();
      // Ptr<ConstantVelocityMobilityModel> mob = DynamicCast<ConstantVelocityMobilityModel>(nodes.Get(i))
      mob->SetVelocity (Vector (20.0, 0.0, 0.0));
      // Vector pos = mob->GetPosition ();
      // std::cout << pos << std::endl;
    }

  InitiateNodeInfo ();

  YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default ();
  YansWavePhyHelper wavePhy = YansWavePhyHelper::Default ();
  wavePhy.SetChannel (waveChannel.Create ());
  wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();
  devices = waveHelper.Install (wavePhy, waveMac, nodes);

  conn_mat.InitiateMatrix (devices.GetN (), devices.GetN ());
  delay_mat.InitiateMatrix (PACKET_NUM*SENDER_NUM, devices.GetN ());
  packetSrc.resize (SENDER_NUM * PACKET_NUM);
  // InitiateRebroadcastMatrix (devices.GetN ());
  for (uint64_t i = 0; i != devices.GetN (); ++i)
    {
      device = DynamicCast<WaveNetDevice> (devices.Get (i));
      device->SetReceiveCallback (MakeCallback (&WaveNetDeviceExample::Receive, this));
      // device->SetWaveVsaCallback(MakeCallback(&WaveNetDeviceExample::ReceiveVsa, this));
    }
  PrintNodeListInfo ();

  // Tracing
  wavePhy.EnablePcap ("wave-simple-device", devices);
}

void
WaveNetDeviceExample::InitiateNodeInfo ()
{
  for (uint64_t i = 0; i != nodes.GetN (); ++i)
    {
      Ptr<ConstantVelocityMobilityModel> nodeMobility =
          nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ();
      ClusterSupport::NodeInfo nodeInfo;
      nodeInfo.degree = ClusterSupport::UN;
      nodeInfo.posX = nodeMobility->GetPosition ().x;
      nodeInfo.posY = nodeMobility->GetPosition ().y;
      nodeInfo.posZ = nodeMobility->GetPosition ().z;
      nodeInfo.velX = nodeMobility->GetVelocity ().x;
      nodeInfo.velY = nodeMobility->GetVelocity ().y;
      nodeInfo.velZ = nodeMobility->GetVelocity ().z;
      nodeInfo.clusterId = nodeMobility->GetObject<Node> ()->GetId ();
      nodeInfoList.push_back (nodeInfo);
    }
}

void
WaveNetDeviceExample::UpdateAllNodeInfo ()
{
  for (uint64_t i = 0; i < nodeInfoList.size (); i++)
    {
      Vector pos = nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ()->GetPosition ();
      Vector vel = nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ()->GetVelocity ();
      std::cout << pos.x << std::endl;
      // nodeInfoList[i].clusterId = i;
      nodeInfoList[i].posX = pos.x;
      nodeInfoList[i].posY = pos.y;
      nodeInfoList[i].posZ = pos.z;
      nodeInfoList[i].velX = vel.x;
      nodeInfoList[i].velY = vel.y;
      nodeInfoList[i].velZ = vel.z;
    }
}

void
WaveNetDeviceExample::UpdateNodeInfo (uint64_t node_id, ClusterSupport::NodeDegree degree,
                                      uint64_t cluster_id, double posX, double posY, double posZ,
                                      double velX, double velY, double velZ)
{
  nodeInfoList[node_id].degree = degree;
  nodeInfoList[node_id].clusterId = cluster_id;
  nodeInfoList[node_id].posX = posX;
  nodeInfoList[node_id].posY = posY;
  nodeInfoList[node_id].posZ = posZ;
  nodeInfoList[node_id].velX = velX;
  nodeInfoList[node_id].velY = velY;
  nodeInfoList[node_id].velZ = velZ;
}
// void
// WaveNetDeviceExample::InitiateRebroadcastMatrix (uint64_t node_num)
// {
//   for (uint64_t i = 0; i < node_num; i++)
//     {
//       rebroadcast_check.push_back (0);
//     }
// }
/*This function for converting Mac address to String*/
std::string
WaveNetDeviceExample::MactoString (const Address &sender)
{
  AddressValue address_val = AddressValue (sender);
  std::string mac_string;
  Ptr<const AttributeChecker> checker = MakeAddressChecker ();
  mac_string = address_val.SerializeToString (checker);
  return mac_string;
}

/*end function*/

/*This function for converting Mac string address to int */
uint64_t
WaveNetDeviceExample::MactoInt (std::string const &s)
{
  unsigned char a[6];
  int last = -1;
  std::string str = s.substr (6);
  int rc = sscanf (str.c_str (), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%n", a + 0, a + 1, a + 2, a + 3,
                   a + 4, a + 5, &last);

  if (rc != 6 || (int) str.size () != last)
    throw std::runtime_error ("invalid mac address format " + str);
  return uint64_t (a[0]) << 40 | uint64_t (a[1]) << 32 |
         (
             // 32-bit instructions take fewer bytes on x86, so use them as much as possible.
             uint64_t (a[2]) << 24 | uint64_t (a[3]) << 16 | uint64_t (a[4]) << 8 |
             uint64_t (a[5]));
}
/*end function*/

// void
// WaveNetDeviceExample::Send (uint64_t channel, uint64_t sender_id,)
// {
//   if (Now ().GetSeconds () < BeaconExTime + 0.1)
//     {
//       Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
//       // const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
//       Mac48Address dest = Mac48Address::GetBroadcast ();

//       // const TxProfile txProfile = TxProfile (SCH1);

//       Ptr<Packet> p = Create<Packet> (100);

//       BeaconExHeader m_header;

//       Vector pos =
//           nodes.Get (sender_id)->GetObject<ConstantVelocityMobilityModel> ()->GetPosition ();
//       Vector vel =
//           nodes.Get (sender_id)->GetObject<ConstantVelocityMobilityModel> ()->GetVelocity ();
//       std::cout << pos.x << std::endl;
//       // double posX = pos.x;
//       // double posY = pos.y;
//       // double posZ = pos.z;
//       // double velX = vel.x;
//       // double velY = vel.y;
//       // double velZ = vel.z;
//       // ClusterSupport::BeaconInfo beaconInfo{sender_id, posX, posY, posZ, velX, velY, velZ};
//       ClusterSupport::BeaconInfo beaconInfo;
//       beaconInfo.nodeId = sender_id;
//       beaconInfo.posX = pos.x;
//       beaconInfo.posY = pos.y;
//       beaconInfo.posZ = pos.z;
//       beaconInfo.velX = vel.x;
//       beaconInfo.velY = vel.y;
//       beaconInfo.velZ = vel.z;
//       m_header.SetBeaconInfo (beaconInfo);
//       p->AddHeader (m_header);
//       // p->DeleteTxProfile (txInfo);
//       // const SchInfo schInfo = SchInfo (channel, false, EXTENDED_CONTINUOUS);
//       // sender->StartSch (schInfo);
//       // sender->RegisterTxProfile (txProfile);
//       sender->Send (p, dest, 0);
//       ScheduleSendPacket (Seconds (0.1), channel, sender_id);
//     }
//   else
//     std::cout << "DONE BEACON EXCHANGE" << std::endl;
// }
// void
// WaveNetDeviceExample::ReBroadcastPacket (uint64_t channel, uint64_t seq, uint64_t sender_id, Ptr<Packet> pkt)
// {
//   Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
//   const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
//   Mac48Address bssWildcard = Mac48Address::GetBroadcast ();
//   const TxInfo txInfo = TxInfo (channel);
//   sender->SendX (pkt, bssWildcard, WSMP_PROT_NUMBER, txInfo);
// }

void
WaveNetDeviceExample::SendPacket (uint64_t channel, uint64_t sender_id, uint64_t process,
                                  uint64_t receiver_id, uint64_t packet_id)
{
  switch (process)
    {
    case 1:
      if (Now ().GetSeconds () < BeaconExTime + 0.1)
        {
          Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
          // const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
          Mac48Address dest = Mac48Address::GetBroadcast ();

          // const TxProfile txProfile = TxProfile (SCH1);

          Ptr<Packet> p = Create<Packet> (100);

          BeaconExHeader m_header;

          Vector pos =
              nodes.Get (sender_id)->GetObject<ConstantVelocityMobilityModel> ()->GetPosition ();
          Vector vel =
              nodes.Get (sender_id)->GetObject<ConstantVelocityMobilityModel> ()->GetVelocity ();
          std::cout << pos.x << std::endl;
          // double posX = pos.x;
          // double posY = pos.y;
          // double posZ = pos.z;
          // double velX = vel.x;
          // double velY = vel.y;
          // double velZ = vel.z;
          // ClusterSupport::BeaconInfo beaconInfo{sender_id, posX, posY, posZ, velX, velY, velZ};
          ClusterSupport::BeaconInfo beaconInfo;
          beaconInfo.nodeId = sender_id;
          beaconInfo.posX = pos.x;
          beaconInfo.posY = pos.y;
          beaconInfo.posZ = pos.z;
          beaconInfo.velX = vel.x;
          beaconInfo.velY = vel.y;
          beaconInfo.velZ = vel.z;
          m_header.SetBeaconInfo (beaconInfo);
          p->AddHeader (m_header);
          // p->DeleteTxProfile (txInfo);
          // const SchInfo schInfo = SchInfo (channel, false, EXTENDED_CONTINUOUS);
          // sender->StartSch (schInfo);
          // sender->RegisterTxProfile (txProfile);
          const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
          const TxInfo txInfo = TxInfo (channel);
          sender->SendX (p, dest, WSMP_PROT_NUMBER, txInfo);
          ScheduleSendPacket (Seconds (0.1), channel, sender_id);
        }
      else
        std::cout << "DONE BEACON EXCHANGE" << std::endl;
      break;
    case 2:
      if (Now ().GetSeconds () < ClusterFormationTime + BeaconExTime + 0.1)
        {
          if (nodeInfoList[sender_id].degree != ClusterSupport::CM)
            {
              nodeInfoList[sender_id].degree = ClusterSupport::CH;
              Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
              Mac48Address dest = Mac48Address::GetBroadcast ();
              Ptr<Packet> p = Create<Packet> (100);
              FormClusterHeader m_header;
              // m_header.SetMobilityInfo (clusterInfo);
              m_header.SetClusterId (sender_id);
              m_header.SetNodeId (sender_id);
              m_header.SetTs ();
              p->AddHeader (m_header);
              // p->DeleteTxProfile (txInfo);
              // const SchInfo schInfo = SchInfo (channel, false, EXTENDED_CONTINUOUS);
              // sender->StartSch (schInfo);
              // sender->RegisterTxProfile (txProfile);
              const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
              const TxInfo txInfo = TxInfo (channel);
              sender->SendX (p, dest, WSMP_PROT_NUMBER, txInfo);
              // SetTwaitArr ();
              ScheduleSendPacket1 (Seconds (0.1), channel, sender_id);
            }
          else
            {
              std::cout << "I am A CM" << std::endl;
            }
        }
      else
        std::cout << "DONE ELECTION" << std::endl;
      break;
    case 3: // check check check check
      if (Now ().GetSeconds () < ClusterFormationTime + BeaconExTime + 0.1 + DataExchangeTime)
        {
          Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
          Ptr<WaveNetDevice> receiver = DynamicCast<WaveNetDevice> (devices.Get (receiver_id));
          Ptr<WaveNetDevice> clusterHead =
              DynamicCast<WaveNetDevice> (devices.Get (nodeInfoList[sender_id].clusterId));
          DataPacketHeader dataHeader;
          dataHeader.SetTs ();
          dataHeader.SetSeq (sender_id);
          dataHeader.SetPacketId (packet_id);
          Ptr<Packet> p = Create<Packet> (100);
          p->AddHeader (dataHeader);
          // sender->Send (p, receiver->GetAddress (), 0);
          const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
          const TxInfo txInfo = TxInfo (channel);
          Mac48Address dest = Mac48Address::GetBroadcast ();
          // if(conn_mat.IsNeighbor(sender_id,receiver_id)){
          //   sender->SendX (p, receiver->GetAddress (), WSMP_PROT_NUMBER,txInfo);
          // }
          // else
          //   sender->SendX (p, clusterHead->GetAddress (), WSMP_PROT_NUMBER, txInfo);
          // std::cout << "Cannot send" << std::endl;
          // const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
          // const TxInfo txInfo = TxInfo (channel);
          sender->SendX (p, dest, WSMP_PROT_NUMBER, txInfo);
        }
      else
        std::cout << "DONE DATA EXCHANGE" << std::endl;
      break;
    default:
      break;
    }
}
void
WaveNetDeviceExample::ScheduleSendPacket (Time time, uint64_t channel, uint64_t sender_id)
{
  Simulator::Schedule (time, &WaveNetDeviceExample::SendPacket, this, channel, sender_id, 1, 0,0);
}
void
WaveNetDeviceExample::ScheduleSendPacket1 (Time time, uint64_t channel, uint64_t sender_id)
{
  Simulator::Schedule (time, &WaveNetDeviceExample::SendPacket, this, channel, sender_id, 2, 0,0);
}

bool
WaveNetDeviceExample::Receive (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode,
                               const Address &sender)
{

  // SeqTsHeader seqTs;
  // pkt->PeekHeader (seqTs);
  Ptr<Packet> p = pkt->Copy ();

  /*Get header*/
  // BeaconExHeader header;
  // p->PeekHeader (header);

  PacketMetadata::ItemIterator iterator = p->BeginItem ();
  PacketMetadata::Item item;
  item = iterator.Next ();
  // const std::string header_type = item.tid.GetName ();
  // switch(header_type){
  //   case "ns3::BeaconExHeader":

  // }
  if (item.tid.GetName () == "ns3::BeaconExHeader")
    {
      BeaconExHeader header;
      p->PeekHeader (header);
      Ptr<Node> node = dev->GetNode ();
      uint64_t node_pos = node->GetId ();
      Ptr<ConstantVelocityMobilityModel> mob = node->GetObject<ConstantVelocityMobilityModel> ();
      // Vector mob_pos = mob->GetPosition ();
      Vector mob_vel = mob->GetVelocity ();
      // double distance = mob->GetDistanceFrom (nodes.Get (rsuId)->GetObject<ConstantVelocityMobilityModel> ());
      uint64_t sender_id = MactoInt (MactoString (sender)) - 1;

      /*Check if that node receive the packet or not*/
      // if (conn_mat.GetElement (seqTs.GetSeq () - 1, node_pos) == 0)
      //   {
      //     delay_mat.InsertEntry (seqTs.GetSeq () - 1, node_pos,
      //                            Now ().GetSeconds () - seqTs.GetTs ().GetSeconds ());
      //     conn_mat.InsertEntry (seqTs.GetSeq () - 1, node_pos, 1);
      //   }
      conn_mat.InsertEntry (sender_id, node_pos, 1);
      // delay_mat.InsertEntry (sender_id, node_pos,
      //                        Now ().GetSeconds () - header.GetTs ().GetSeconds ());

      std::cout << "node id:" << node_pos << std::endl; //print node received packet
      std::cout << "receive packet: " << std::endl
                << "  sendTime = " << header.GetTs ().GetSeconds () << "s," << std::endl
                << "  recvTime = " << Now ().GetSeconds () << "s," << std::endl
                << "  velovity = " << mob_vel << "m/s" << std::endl
                << "  Sender ID: " << sender_id << std::endl
                << "  Sender pos: "
                << "(" << header.GetBeaconInfo ().posX << "," << header.GetBeaconInfo ().posY << ","
                << header.GetBeaconInfo ().posZ << ")" << std::endl
                << "  Sender vel: "
                << "(" << header.GetBeaconInfo ().velX << "," << header.GetBeaconInfo ().velY << ","
                << header.GetBeaconInfo ().velZ << ")" << std::endl;
    }
  else if (item.tid.GetName () == "ns3::FormClusterHeader")
    {
      FormClusterHeader m_header;
      p->PeekHeader (m_header);
      Ptr<Node> node = dev->GetNode ();
      uint64_t node_pos = node->GetId ();
      // ClusterSupport::NodeInfo clusterInfo = m_header.GetMobilityInfo ();
      Ptr<ConstantVelocityMobilityModel> mob = node->GetObject<ConstantVelocityMobilityModel> ();
      Vector mob_pos = mob->GetPosition ();
      Vector mob_vel = mob->GetVelocity ();
      if (nodeInfoList[node_pos].degree == ClusterSupport::UN)
        {

          nodeInfoList[node_pos].degree = ClusterSupport::CM;
          nodeInfoList[node_pos].clusterId = m_header.GetClusterId ();
          UpdateNodeInfo (node_pos, ClusterSupport::CM, m_header.GetNodeId (), mob_pos.x, mob_pos.y,
                          mob_pos.z, mob_vel.x, mob_vel.y, mob_vel.z);
          std::cout << "Receive formation msg: " << std::endl
                    << "  receiver: " << node_pos << std::endl
                    << "  sender: " << m_header.GetNodeId () << std::endl
                    << "  sendTime: " << m_header.GetTs ().GetSeconds () << "s," << std::endl
                    << "  recvTime: " << Now ().GetSeconds () << "s," << std::endl
                    << "  Cluster id: " << m_header.GetClusterId () << std::endl
                    << "  recv state: " << nodeInfoList[node_pos].degree << std::endl;
        }
      else
        {
          // Simulator::Cancel (&WaveNetDeviceExample::Send;
          std::cout
              << "Node " << node_pos << " already a CM/CH. Receive formation msg: "
              << std::endl
              // // << "  receiver: " << node_pos << std::endl
              // // << "  sender: " << clusterInfo.nodeId << std::endl
              // << "  sendTime: " << m_header.GetTs ().GetSeconds () << "s," << std::endl
              // << "  recvTime: " << Now ().GetSeconds () << "s,"
              // << std::endl
              // // << "  Sender pos: "
              // // << "(" << clusterInfo.posX << "," << clusterInfo.posY << "," << clusterInfo.posZ
              // // << ")" << std::endl
              // // << "  Sender vel: "
              // // << "(" << clusterInfo.velX << "," << clusterInfo.velY << "," << clusterInfo.velZ
              // // << ")" << std::endl
              << "  Cluster id: " << m_header.GetClusterId () << std::endl
              << "  recver state: " << nodeInfoList[node_pos].degree << std::endl;
        }
    }
  else
    {
      DataPacketHeader dataHeader;
      p->PeekHeader (dataHeader);
      Ptr<Node> node = dev->GetNode ();
      uint64_t node_pos = node->GetId ();
      uint64_t sender_id = MactoInt (MactoString (sender)) - 1;
      if ((node_pos != dataHeader.GetSeq ())&& (delay_mat.GetElement(dataHeader.GetPacketId(),node_pos)== 0))
        {
          delay_mat.InsertEntry (dataHeader.GetPacketId (), node_pos,
                                 Now ().GetSeconds () - dataHeader.GetTs ().GetSeconds ());
          std::cout << "Receive data packet: " << std::endl
                    << " Src: " << dataHeader.GetSeq () << std::endl
                    << " SendTime: " << dataHeader.GetTs ().GetSeconds () << std::endl
                    << " Receive time: " << Now ().GetSeconds () << "s" << std::endl
                    << " Sender id: " << sender_id << std::endl
                    <<" Packet id: "<<dataHeader.GetPacketId()<<std::endl
                    << " Receive node: " << node_pos << std::endl;
        }

      if ((nodeInfoList[node_pos].degree == ClusterSupport::CH) )
        {
          Simulator::Schedule (Seconds (Now ().GetSeconds ()),
                               &WaveNetDeviceExample::ReBroadcastPacket, this, SCH1, node_pos, p);
          std::cout << " IAM CH" << std::endl;
          // rebroadcast_count++;
        }
    }
  // else
  //   std::cout << "Unknow package" << std::endl;

  return true;
}
void
WaveNetDeviceExample::ReBroadcastPacket (uint32_t channel, uint64_t sender_id, Ptr<Packet> pkt)
{
  Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
  const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
  Mac48Address bssWildcard = Mac48Address::GetBroadcast ();
  const TxInfo txInfo = TxInfo (channel);
  sender->SendX (pkt, bssWildcard, WSMP_PROT_NUMBER, txInfo);
}
void
WaveNetDeviceExample::SendPacketExample ()
{
  Packet::EnablePrinting ();
  CreateWaveNodeCustomMobility ("position.txt");

  const SchInfo schInfo = SchInfo (SCH1, false, EXTENDED_ALTERNATING);
  // Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, sender, schInfo);
  // const TxProfile txProfile = TxProfile (SCH1);
  // Ptr<WaveNetDevice> sender;
  // Ptr<WaveNetDevice> receiver;
  for (uint64_t i = 0; i != devices.GetN (); ++i)
    {
      Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (i));
      Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, sender, schInfo);
      Ptr<WaveNetDevice> receiver = DynamicCast<WaveNetDevice> (devices.Get (i));
      Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, receiver, schInfo);
      // Simulator::Schedule (Seconds (0.1), &WaveNetDevice::RegisterTxProfile, sender, txProfile);
      Simulator::Schedule (Seconds (0.1 + 0.002 * (i + 1)), &WaveNetDeviceExample::SendPacket, this,
                           SCH1, i, 1, 0,0);
      Simulator::Schedule (
          Seconds (BeaconExTime + ClusterFormationTime + 0.1 + DataExchangeTime),
          &WaveNetDevice::StopSch, sender, SCH1);
      Simulator::Schedule (
          Seconds (BeaconExTime + ClusterFormationTime + 0.1 + DataExchangeTime),
          &WaveNetDevice::StopSch, receiver, SCH1);
    }

  // for (uint64_t i = 0; i != devices.GetN (); ++i)
  //   {
  //     Ptr<WaveNetDevice> receiver = DynamicCast<WaveNetDevice> (devices.Get (i));
  //     Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, receiver, schInfo);
  //   }

  // for (uint64_t i = 0; i != devices.GetN (); ++i)
  //   {
  //     Simulator::Schedule (Seconds (0.1 + 0.02 * (i + 1)), &WaveNetDeviceExample::SendPacket, this,
  //                          SCH1, i, 1, 0);
  //   }

  Simulator::Schedule (Seconds (0.4), &WaveNetDeviceExample::SetTwaitArr, this);
  // conn_mat.Print1D (TwaitArr);
  // Simulator::Schedule (Seconds (0.4 ), &WaveNetDeviceExample::Send, this, SCH1, 0);
  SetTwaitArr ();
  // std::cout << TwaitArr[0] << std::endl;
  // Simulator::Schedule (Seconds (0.4 + TwaitArr[0]), &WaveNetDeviceExample::Send, this, SCH1, 0, 2);

  for (uint64_t i = 0; i != devices.GetN (); ++i)
    {
      Simulator::Schedule (Seconds (0.4 + TwaitArr[i]), &WaveNetDeviceExample::SendPacket, this,
                           SCH1, i, 2, 0,0);
    }
  // SendFormation ();
  // Simulator::Schedule (Seconds (BeaconExTime + ClusterFormationTime + 0.1 + 0.1),
  //                      &WaveNetDeviceExample::SendPacket, this, SCH1, 1, 3, 2);

  for (uint64_t i = 0; i < PACKET_NUM; i++)
    {
      Simulator::Schedule (Seconds (BeaconExTime + ClusterFormationTime + 0.1 + (0.1 * i)),
                           &WaveNetDeviceExample::SendPacket, this, SCH1, 0, 3, 0, i);
      packetSrc[i] = 0;
      Simulator::Schedule (Seconds (BeaconExTime + ClusterFormationTime + 0.1+0.001 + (0.1 * i)),
                           &WaveNetDeviceExample::SendPacket, this, SCH1, 1, 3, 1, i + PACKET_NUM);
      packetSrc[i + PACKET_NUM] = 1;
    }
  // for (uint64_t i = 0; i != devices.GetN (); ++i)
  //   {
  //     Simulator::Schedule (Seconds (BeaconExTime + ClusterFormationTime + 0.1 + DataExchangeTime),
  //                          &WaveNetDevice::DeleteTxProfile,
  //                          DynamicCast<WaveNetDevice> (devices.Get (i)), SCH1);
  //     Simulator::Schedule (Seconds (BeaconExTime + ClusterFormationTime + 0.1 + DataExchangeTime),
  //                          &WaveNetDevice::StopSch, DynamicCast<WaveNetDevice> (devices.Get (i)),
  //                          SCH1);
  //     Simulator::Schedule (Seconds (BeaconExTime + ClusterFormationTime + 0.1 + DataExchangeTime),
  //                          &WaveNetDevice::StopSch, DynamicCast<WaveNetDevice> (devices.Get (i)),
  //                          SCH1);
  //   }

  Simulator::Stop (Seconds (1 + BeaconExTime + ClusterFormationTime + DataExchangeTime));
  Simulator::Run ();

  // PrintNodeListInfo ();
  UpdateAllNodeInfo ();
  PrintNodeListInfo ();
  conn_mat.PrintMatrix ();
  SetTwaitArr ();
  Simulator::Destroy ();

  // conn_mat.PrintMatrix ();
  // std::cout << std::endl;
  // conn_mat.GetNeighbors ();
  // std::cout << std::endl;
  delay_mat.SetNeighborMatrix ();
  delay_mat.PrintMatrix ();
  delay_mat.SetDelayIdxVec (packetSrc, SENDER_NUM);
  std::cout << "The total avg delay index: " << delay_mat.GetTotalAvgDelayIdx () << std::endl;
  delay_mat.GetTotalPDR (packetSrc, SENDER_NUM);
  for (uint64_t i = 0; i < packetSrc.size (); i++)
    {
      std::cout << "Packet # " << i << "  Src: " << packetSrc[i] << std::endl;
    }
  // delay_mat.PrintMatrix ();
  // delay_mat.GetDelayIndexes ();
  // float min_delay_index = delay_mat.GetMinDelayIndex ();
  // std::cout << min_delay_index << std::endl;

  // double Twait = CalculateWaitingTime (node_pos, nodes.GetN (), distance);
  
}
void
WaveNetDeviceExample::PrintNodeListInfo ()
{
  for (uint64_t i = 0; i < nodeInfoList.size (); i++)
    {
      std::cout << "Node " << i << std::endl
                << "state: " << nodeInfoList[i].degree << std::endl
                << "cluster id: " << nodeInfoList[i].clusterId << std::endl
                << "position: "
                << "(" << nodeInfoList[i].posX << "," << nodeInfoList[i].posY << ","
                << nodeInfoList[i].posZ << ")" << std::endl
                << "velocity: "
                << "(" << nodeInfoList[i].velX << "," << nodeInfoList[i].velY << ","
                << nodeInfoList[i].velZ << ")" << std::endl
                << "---------------------------------------------------" << std::endl;
    }
}
double
WaveNetDeviceExample::CalculateDistance (uint64_t node1, uint64_t node2)
{
  Ptr<ConstantVelocityMobilityModel> mob1 =
      nodes.Get (node1)->GetObject<ConstantVelocityMobilityModel> ();
  Ptr<ConstantVelocityMobilityModel> mob2 =
      nodes.Get (node2)->GetObject<ConstantVelocityMobilityModel> ();
  double distance = mob1->GetDistanceFrom (mob2);
  return distance;
}

double
WaveNetDeviceExample::CalculateWaitingTime (uint32_t node_id, uint32_t node_num, double distance)
{
  // std::cout << node_id << std::endl;
  // std::cout << conn_mat.GetNeighborNum (0) << std::endl;
  // std::cout << nodes.GetN() << std::endl;
  conn_mat.SetNeighborMatrix ();
  double centrality_idx = (double) conn_mat.GetNeighborNum (node_id) / node_num;
  // std::cout << centrality_idx << std::endl;
  double distance_idx = (double) (Communication_range - distance) / Communication_range;
  double conn_idx = (double) (centrality_idx + distance_idx) / 2;
  double Twait = (double) (1 - conn_idx) * Twaitmax;
  return Twait;
}
void
WaveNetDeviceExample::SetTwaitArr ()
{
  std::cout << "At " << Now ().GetSeconds () << "s" << std::endl;
  for (uint64_t i = 0; i < nodes.GetN (); i++)
    {
      double distance = CalculateDistance (i, rsuId);
      double Twait = CalculateWaitingTime (i, nodes.GetN (), distance);
      TwaitArr.push_back (Twait);
      std::cout << "Node " << i << ":" << Twait << std::endl;
    }
}

// void
// WaveNetDeviceExample::SendFormation ()
// {
//   for (uint64_t i = 0; i != devices.GetN (); ++i)
//     {
//       Simulator::Schedule (Seconds (0.4+ TwaitArr[i]), &WaveNetDeviceExample::Send, this, SCH1, i, 2);
//     }
// }

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  WaveNetDeviceExample example;
  std::cout << "run WAVE WSMP routing service case:" << std::endl;
  example.SendPacketExample ();

  return 0;
}