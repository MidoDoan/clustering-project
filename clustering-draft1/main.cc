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
#include <algorithm>
#include <map>

#define REBRCT_NUM 3
#define rsuId 2
#define Communication_range 50
#define Twaitmax 1
#define BeaconExTime 5
#define ClusterFormationTime 5
#define DataExchangeTime 30
#define LANE_NUM 3
#define LANE_WIDTH 2
#define t0 0.7
using namespace ns3;

class ClusteringProtocol
{
public:
  ClusteringProtocol (uint64_t node_num, uint64_t sender_num, uint64_t packet_num)
  {
    NODE_NUM = node_num;
    SENDER_NUM = sender_num;
    PACKET_NUM = packet_num;
  }
  void SendPacketExample (void);

private:
  bool Receive (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender);

  void CreateWaveNodeCustomMobility (std::string file_path);
  std::string MactoString (Address const &sender);
  uint64_t MactoInt (std::string const &s);
  NodeContainer nodes; ///< the nodes
  NetDeviceContainer devices; ///< the devices
  Ptr<WaveNetDevice> device;

  void SendPacket (uint64_t channel, uint64_t sender_id, uint64_t process, uint64_t receiver_id,
                   uint64_t packet_id);

  ConnectivityMatrix conn_mat;
  ConnectivityMatrix delay_mat;
  double CalculateWaitingTime (uint32_t node_id, uint32_t node_num, double distance);
  double CalculateDistance (uint64_t node1, uint64_t node2);
  std::vector<double> TwaitArr;
  std::vector<ClusterSupport::NodeInfo> nodeInfoList;
  void ScheduleSendPacket (Time time, uint64_t channel, uint64_t sender_id, uint64_t process,
                           uint64_t packet_id);
  void InitiateNodeInfo ();
  void UpdateAllNodeInfo ();
  void UpdateNodeInfo (uint64_t node_id, ClusterSupport::NodeDegree degree, uint64_t cluster_id,
                       double posX, double posY, double posZ, double velX, double velY,
                       double velZ);
  void PrintNodeListInfo ();
  void SetTwaitArr ();
  void ReBroadcastPacket (uint32_t channel, uint64_t sender_id, Ptr<Packet> pkt);
  std::vector<uint64_t> packetSrc;

  void ScheduleProcess (uint64_t process, uint64_t sender_id, uint64_t channel, uint64_t packet_id);
  uint64_t NODE_NUM;
  uint64_t SENDER_NUM;
  uint64_t PACKET_NUM;
  std::vector<uint64_t> clusterId;
  std::map<uint64_t, std::vector<uint64_t>> clusterList;
  void SetClusterIdList ();
  void SetClusterList ();
  void PrintClusterList ();
};
void
ClusteringProtocol::SetClusterIdList ()
{
  // Save Cluster Id of nodes in NodeListInfo to a vector
  std::vector<uint64_t> clusterIdVec; // contain cluster id which can be duplicated
  for (uint64_t i = 0; i < nodeInfoList.size (); i++)
    {
      clusterIdVec.push_back (nodeInfoList[i].clusterId);
    }
  std::vector<uint64_t>::iterator ip;

  // Sorting the array
  std::sort (clusterIdVec.begin (), clusterIdVec.end ());

  // Using std::unique
  ip = std::unique (clusterIdVec.begin (), clusterIdVec.begin () + clusterIdVec.size ());

  // Resizing the vector so as to remove the undefined terms
  clusterIdVec.resize (std::distance (clusterIdVec.begin (), ip));
  clusterId = clusterIdVec;

  // Displaying the vector after applying std::unique
  std::cout << "Cluster id: ";
  for (uint64_t i = 0; i < clusterId.size (); i++)
    {
      std::cout << clusterId[i] << " ";
    }
  std::cout << std::endl;
}
void
ClusteringProtocol::SetClusterList ()
{
  for (uint64_t i = 0; i < clusterId.size (); i++)
    {
      std::vector<uint64_t> clusterMember;
      for (uint64_t j = 0; j < nodeInfoList.size (); j++)
        {

          if (nodeInfoList[j].clusterId == clusterId[i])
            {
              clusterMember.push_back (j);
            }
        }
      clusterList.insert (std::pair<uint64_t, std::vector<uint64_t>> (clusterId[i], clusterMember));
    }
}
void
ClusteringProtocol::PrintClusterList ()
{
  SetClusterIdList ();
  SetClusterList ();
  std::cout << "CH \t CM" << std::endl;
  for (std::map<uint64_t, std::vector<uint64_t>>::const_iterator it = clusterList.begin ();
       it != clusterList.end (); ++it)
    {
      std::cout << it->first << ":\t ";
      for (std::vector<uint64_t>::const_iterator b = it->second.begin (); b != it->second.end ();
           ++b)
        {
          std::cout << *b << " ";
        }
      std::cout << std::endl;
    }
}

void
ClusteringProtocol::CreateWaveNodeCustomMobility (std::string file_path)
{
  Position posInfo = Position (file_path);
  nodes = posInfo.GetContainer (LANE_NUM, LANE_WIDTH, NODE_NUM);

  std::cout << "The number of nodes is:" << nodes.GetN () << std::endl;

  for (uint64_t i = 0; i != nodes.GetN (); ++i)
    {
      Ptr<ConstantVelocityMobilityModel> mob =
          nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ();
      mob->SetVelocity (Vector (20.0, 0.0, 0.0));
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
  delay_mat.InitiateMatrix (PACKET_NUM * SENDER_NUM, devices.GetN ());
  packetSrc.resize (SENDER_NUM * PACKET_NUM);
  for (uint64_t i = 0; i != devices.GetN (); ++i)
    {
      device = DynamicCast<WaveNetDevice> (devices.Get (i));
      device->SetReceiveCallback (MakeCallback (&ClusteringProtocol::Receive, this));
    }
  PrintNodeListInfo ();

  // Tracing
  wavePhy.EnablePcap ("wave-simple-device", devices);
}

void
ClusteringProtocol::InitiateNodeInfo ()
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
ClusteringProtocol::UpdateAllNodeInfo ()
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
ClusteringProtocol::UpdateNodeInfo (uint64_t node_id, ClusterSupport::NodeDegree degree,
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

/*This function for converting Mac address to String*/
std::string
ClusteringProtocol::MactoString (const Address &sender)
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
ClusteringProtocol::MactoInt (std::string const &s)
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

void
ClusteringProtocol::SendPacket (uint64_t channel, uint64_t sender_id, uint64_t process,
                                  uint64_t receiver_id, uint64_t packet_id)
{
  switch (process)
    {
    case 1:
      if (Now ().GetSeconds () < BeaconExTime)
        {
          Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
          Mac48Address dest = Mac48Address::GetBroadcast ();

          Ptr<Packet> p = Create<Packet> (100);

          BeaconExHeader m_header;

          Vector pos =
              nodes.Get (sender_id)->GetObject<ConstantVelocityMobilityModel> ()->GetPosition ();
          Vector vel =
              nodes.Get (sender_id)->GetObject<ConstantVelocityMobilityModel> ()->GetVelocity ();
          std::cout << pos.x << std::endl;

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

          const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
          const TxInfo txInfo = TxInfo (channel);
          sender->SendX (p, dest, WSMP_PROT_NUMBER, txInfo);
          ScheduleSendPacket (Seconds (0.1), channel, sender_id, process, 0);
        }
      else
        std::cout << "DONE BEACON EXCHANGE" << std::endl;
      break;
    case 2:
      if (Now ().GetSeconds () < ClusterFormationTime + BeaconExTime)
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

              const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
              const TxInfo txInfo = TxInfo (channel);
              sender->SendX (p, dest, WSMP_PROT_NUMBER, txInfo);
              // SetTwaitArr ();
              ScheduleSendPacket (Seconds (0.1), channel, sender_id, process, 0);
            }
          else
            {
              std::cout << "I am A CM" << std::endl;
            }
        }
      else
        std::cout << "DONE ELECTION" << std::endl;

      break;
    case 3:

      if (Now ().GetSeconds () < ClusterFormationTime + BeaconExTime + DataExchangeTime + t0)
        {

          Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
          Ptr<WaveNetDevice> clusterHead =
              DynamicCast<WaveNetDevice> (devices.Get (nodeInfoList[sender_id].clusterId));
          DataPacketHeader dataHeader;
          dataHeader.SetTs ();
          dataHeader.SetSeq (sender_id);
          dataHeader.SetPacketId (packet_id);
          Ptr<Packet> p = Create<Packet> (100);
          Mac48Address dest = Mac48Address::GetBroadcast ();
          p->AddHeader (dataHeader);
          // sender->Send (p, receiver->GetAddress (), 0);
          const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
          const TxInfo txInfo = TxInfo (channel);
          std::cout << Now ().GetSeconds () << " " << sender_id << std::endl;
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
ClusteringProtocol::ScheduleProcess (uint64_t process, uint64_t sender_id, uint64_t channel,
                                      uint64_t packet_id)
{
  switch (process)
    {
    case 1:
      ClusteringProtocol::ScheduleSendPacket (Seconds (0.0), channel, sender_id, process,
                                                packet_id);
      break;
    case 2:
      SetTwaitArr ();
      ClusteringProtocol::ScheduleSendPacket (Seconds (TwaitArr[sender_id]), channel, sender_id,
                                                process, packet_id);
      break;
    case 3:
      ClusteringProtocol::ScheduleSendPacket (Seconds (BeaconExTime + ClusterFormationTime), SCH1,
                                                sender_id, process, packet_id);
      break;
    }
}

void
ClusteringProtocol::ScheduleSendPacket (Time time, uint64_t channel, uint64_t sender_id,
                                          uint64_t process, uint64_t packet_id)
{
  Simulator::Schedule (time, &ClusteringProtocol::SendPacket, this, channel, sender_id, process,
                       0, packet_id);
}

bool
ClusteringProtocol::Receive (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode,
                               const Address &sender)
{

  Ptr<Packet> p = pkt->Copy ();

  PacketMetadata::ItemIterator iterator = p->BeginItem ();
  PacketMetadata::Item item;
  item = iterator.Next ();

  if (item.tid.GetName () == "ns3::BeaconExHeader")
    {
      BeaconExHeader header;
      p->PeekHeader (header);
      Ptr<Node> node = dev->GetNode ();
      uint64_t node_pos = node->GetId ();
      Ptr<ConstantVelocityMobilityModel> mob = node->GetObject<ConstantVelocityMobilityModel> ();
      Vector mob_vel = mob->GetVelocity ();
      uint64_t sender_id = MactoInt (MactoString (sender)) - 1;

      conn_mat.InsertEntry (sender_id, node_pos, 1);

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
          std::cout << "Node " << node_pos
                    << " already a CM/CH. Receive formation msg: " << std::endl

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
      if ((node_pos != dataHeader.GetSeq ()) &&
          (delay_mat.GetElement (dataHeader.GetPacketId (), node_pos) == 0))
        {
          delay_mat.InsertEntry (dataHeader.GetPacketId (), node_pos,
                                 Now ().GetSeconds () - dataHeader.GetTs ().GetSeconds ());
          std::cout << "Receive data packet: " << std::endl
                    << " Src: " << dataHeader.GetSeq () << std::endl
                    << " SendTime: " << dataHeader.GetTs ().GetSeconds () << std::endl
                    << " Receive time: " << Now ().GetSeconds () << "s" << std::endl
                    << " Sender id: " << sender_id << std::endl
                    << " Packet id: " << dataHeader.GetPacketId () << std::endl
                    << " Receive node: " << node_pos << std::endl;
        }

      if ((nodeInfoList[dataHeader.GetSeq ()].clusterId == node_pos))
        {

          Simulator::Schedule (Seconds (0.05), &ClusteringProtocol::ReBroadcastPacket, this, SCH1,
                               node_pos, p);
          std::cout << " IAM CH" << std::endl;
        }
    }

  return true;
}
void
ClusteringProtocol::ReBroadcastPacket (uint32_t channel, uint64_t sender_id, Ptr<Packet> pkt)
{
  Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
  const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
  Mac48Address bssWildcard = Mac48Address::GetBroadcast ();
  const TxInfo txInfo = TxInfo (channel);
  sender->SendX (pkt, bssWildcard, WSMP_PROT_NUMBER, txInfo);
}
void
ClusteringProtocol::SendPacketExample ()
{
  Packet::EnablePrinting ();
  CreateWaveNodeCustomMobility ("position.txt");

  const SchInfo schInfo = SchInfo (SCH1, false, EXTENDED_ALTERNATING);

  for (uint64_t i = 0; i != devices.GetN (); ++i)
    {
      Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (i));
      Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, sender, schInfo);
      Ptr<WaveNetDevice> receiver = DynamicCast<WaveNetDevice> (devices.Get (i));
      Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, receiver, schInfo);
      // Simulator::Schedule (Seconds (0.1), &WaveNetDevice::RegisterTxProfile, sender, txProfile);
      Simulator::Schedule (Seconds (0.01 * (i + 1)), &ClusteringProtocol::ScheduleProcess, this, 1,
                           i, SCH1, 0);

      Simulator::Schedule (Seconds (BeaconExTime + ClusterFormationTime + DataExchangeTime + 0.06),
                           &WaveNetDevice::StopSch, sender, SCH1);
      Simulator::Schedule (Seconds (BeaconExTime + ClusterFormationTime + DataExchangeTime + 0.06),
                           &WaveNetDevice::StopSch, receiver, SCH1);
    }

  for (uint64_t j = 0; j != devices.GetN (); ++j)
    {
      Simulator::Schedule (Seconds (BeaconExTime), &ClusteringProtocol::ScheduleProcess, this, 2,
                           j, SCH1, 0);
    }

  for (uint64_t t = 0; t < PACKET_NUM; t++)
    {
      switch (SENDER_NUM)
        {
        case 1:
          Simulator::Schedule (Seconds (t0 + (0.1 * t)), &ClusteringProtocol::ScheduleProcess,
                               this, 3, 0, SCH1, t);
          packetSrc[t] = 0;
          break;
        case 2:
          Simulator::Schedule (Seconds (t0 + (0.1 * t)), &ClusteringProtocol::ScheduleProcess,
                               this, 3, 0, SCH1, t);
          packetSrc[t] = 0;
          Simulator::Schedule (Seconds (t0 + 0.05 + (0.1 * t)),
                               &ClusteringProtocol::ScheduleProcess, this, 3, 1, SCH1,
                               t + PACKET_NUM);
          packetSrc[t + PACKET_NUM] = 1;
          break;
        default:
          break;
        }
    }

  Simulator::Stop (Seconds (t0 + BeaconExTime + ClusterFormationTime + DataExchangeTime));
  Simulator::Run ();

  UpdateAllNodeInfo ();
  PrintNodeListInfo ();
  conn_mat.PrintMatrix ();
  SetTwaitArr ();
  Simulator::Destroy ();

  delay_mat.SetNeighborMatrix ();
  delay_mat.PrintMatrix ();
  delay_mat.SetDelayIdxVec (packetSrc, SENDER_NUM);
  std::cout << "The total avg delay index: " << delay_mat.GetTotalAvgDelayIdx () << std::endl;
  delay_mat.GetTotalPDR (packetSrc, SENDER_NUM);
  for (uint64_t i = 0; i < packetSrc.size (); i++)
    {
      std::cout << "Packet # " << i << "  Src: " << packetSrc[i] << std::endl;
    }
  PrintClusterList ();
}
void
ClusteringProtocol::PrintNodeListInfo ()
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
ClusteringProtocol::CalculateDistance (uint64_t node1, uint64_t node2)
{
  Ptr<ConstantVelocityMobilityModel> mob1 =
      nodes.Get (node1)->GetObject<ConstantVelocityMobilityModel> ();
  Ptr<ConstantVelocityMobilityModel> mob2 =
      nodes.Get (node2)->GetObject<ConstantVelocityMobilityModel> ();
  double distance = mob1->GetDistanceFrom (mob2);
  return distance;
}

double
ClusteringProtocol::CalculateWaitingTime (uint32_t node_id, uint32_t node_num, double distance)
{
  double centrality_idx = (double) conn_mat.GetNeighborNum1 (node_id) / (node_num - 1);

  double conn_idx = (double) (centrality_idx);
  double Twait = (double) (1 - conn_idx) * Twaitmax;
  return Twait;
}
void
ClusteringProtocol::SetTwaitArr ()
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

int
main (int argc, char *argv[])
{
  CommandLine cmd;

  uint64_t node_num;
  uint64_t sender_num;
  uint64_t packet_num;

  cmd.AddValue ("NODE_NUM", "number of node", node_num);
  cmd.AddValue ("SENDER_NUM", "number of sender", sender_num);
  cmd.AddValue ("PACKET_NUM", "number of packet", packet_num);

  cmd.Parse (argc, argv);
  std::cout << node_num << std::endl;
  ClusteringProtocol example (node_num, sender_num, packet_num);
  std::cout << "run WAVE WSMP routing service case:" << std::endl;
  example.SendPacketExample ();

  return 0;
}