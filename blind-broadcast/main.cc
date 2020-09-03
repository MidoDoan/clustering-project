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
#define Communication_range 50
// #define NODE_NUM 10
#define LANE_NUM 3
#define LANE_WIDTH 2
// #define PACKET_NUM 1
// #define SENDER_NUM 1
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
  WaveNetDeviceExample (uint64_t node_num, uint64_t sender_num, uint64_t packet_num, uint64_t mode)
  {
    NODE_NUM = node_num;
    SENDER_NUM = sender_num;
    PACKET_NUM = packet_num;
    MODE = mode;
  }
  void SendPacket (void);

private:
  

  bool Receive (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode, const Address &sender);
  /**
   * Receive VSA function
   * \param pkt the packet
   * \param address the address
   * \returns true if successful
   */
  
  void CreateWaveNodes (void);
  void CreateWaveNodeCustomMobility (std::string file_path);
  std::string MactoString (Address const &sender);
  uint64_t MactoInt (std::string const &s);
  NodeContainer nodes; ///< the nodes
  NetDeviceContainer devices; ///< the devices

  std::vector<std::vector<uint32_t>> rebroadcast_check;
  void InitiateRebroadcastMatrix (uint32_t node_num, uint32_t packet_num);
  void ReBroadcastPacket (uint32_t channel, uint32_t seq, uint32_t sender_id, Ptr<Packet> pkt);
  void SendPacketCustomHeader (uint32_t channel, uint64_t seq, uint32_t sender_id,
                               uint64_t packet_id);
  ConnectivityMatrix conn_mat;
  ConnectivityMatrix delay_mat;
  std::vector<std::vector<uint64_t>> packetList ;
  std::vector < uint64_t > packetSrc;
  void ScheduleSendPacket (Time time, uint64_t channel, uint64_t sender_id,
                           uint64_t packet_count);
  uint64_t NODE_NUM;
  uint64_t SENDER_NUM;
  uint64_t PACKET_NUM;
  uint64_t MODE;

  // double delay_arr[getDeviceNum()];
};
/*My Function*/
void
WaveNetDeviceExample::CreateWaveNodeCustomMobility (std::string file_path)
{
  Position posInfo = Position (file_path);
  nodes = posInfo.GetContainer (LANE_NUM, LANE_WIDTH, NODE_NUM);
  std::cout << nodes.GetN () << std::endl;

  
  for (uint32_t i = 0; i != nodes.GetN (); ++i)
    {
      Ptr<ConstantVelocityMobilityModel> mob =
          nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ();
      // Ptr<ConstantVelocityMobilityModel> mob = DynamicCast<ConstantVelocityMobilityModel>(nodes.Get(i))
      mob->SetVelocity (Vector (20, 0.0, 0.0));
    }

  YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default ();
  YansWavePhyHelper wavePhy = YansWavePhyHelper::Default ();
  wavePhy.SetChannel (waveChannel.Create ());
  wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();
  devices = waveHelper.Install (wavePhy, waveMac, nodes);

  conn_mat.InitiateMatrix (PACKET_NUM*SENDER_NUM, devices.GetN ());
  delay_mat.InitiateMatrix (PACKET_NUM * SENDER_NUM, devices.GetN ());
  packetSrc.resize (PACKET_NUM * SENDER_NUM);
  InitiateRebroadcastMatrix (devices.GetN (), PACKET_NUM*SENDER_NUM);
  for (uint32_t i = 0; i != devices.GetN (); ++i)
    {
      Ptr<WaveNetDevice> device = DynamicCast<WaveNetDevice> (devices.Get (i));
      device->SetReceiveCallback (MakeCallback (&WaveNetDeviceExample::Receive, this));
      // device->SetWaveVsaCallback(MakeCallback(&WaveNetDeviceExample::ReceiveVsa, this));
    }

  // Tracing
  wavePhy.EnablePcap ("wave-simple-device", devices);
}

bool
WaveNetDeviceExample::Receive (Ptr<NetDevice> dev, Ptr<const Packet> pkt, uint16_t mode,
                               const Address &sender)
{
  // SeqTsHeader seqTs;
  // pkt->PeekHeader (seqTs);
  Ptr<Packet> p = pkt->Copy ();

  /*Get header*/
  MyHeader header;
  p->PeekHeader (header);

  Ptr<Node> node = dev->GetNode ();
  uint64_t node_pos = node->GetId ();
  Ptr<ConstantVelocityMobilityModel> mob = node->GetObject<ConstantVelocityMobilityModel> ();
  // Vector mob_pos = mob->GetPosition ();
  Vector mob_vel = mob->GetVelocity ();
  Vector mob_pos = mob->GetPosition ();

  uint32_t sender_id = MactoInt (MactoString (sender)) - 1;
  packetSrc.resize (PACKET_NUM*SENDER_NUM);
  // packetSrc[header.GetPacketId ()] = header.GetSeq ();
  /*Check if that node receive the packet or not*/
  if ((conn_mat.GetElement (header.GetPacketId (), node_pos) == 0) &&
      (node_pos != header.GetSeq ()))
    {
      delay_mat.InsertEntry (header.GetPacketId(), node_pos,
                             Now ().GetSeconds () - header.GetTs ().GetSeconds ());
      conn_mat.InsertEntry (header.GetPacketId (), node_pos, 1);
      std::cout << "node id:" << node_pos << std::endl; //print node received packet
      std::cout << "receive a packet: " << std::endl
                << "  sequence = " << header.GetSeq () << "," << std::endl
                << "  sendTime = " << header.GetTs ().GetSeconds () << "s," << std::endl
                << "  recvTime = " << Now ().GetSeconds () << "s," << std::endl
                << "  velovity = " << mob_vel << "m/s" << std::endl
                << "  pos = " << mob_pos
                << std::endl
                // << "  protocol = 0x" << std::hex << mode << std::dec << std::endl
                // << "  position = " << mob_pos << std::endl
                // << "  Address sender: " << sender << std::endl
                // << "  Sender address: " << MactoString (sender) << std::endl
                << "  PacketId: " << header.GetPacketId () << std::endl
                << "  Sender ID: " << sender_id << std::endl;

      
    }
  // conn_mat.InsertEntry (sender_id, node_pos, 1);
  // delay_mat.InsertEntry (sender_id, node_pos, Now ().GetSeconds () - header.GetTs ().GetSeconds ());
  /*This function is for rebroadcast packet*/
  if(MODE==2){
    if ((rebroadcast_check[header.GetPacketId ()][node_pos] < REBRCT_NUM) &&
        (node_pos != header.GetSeq ()))
      {
        std::cout << "Begin forwarding packet:" << std::endl;

        Simulator::Schedule (Seconds (0.05), &WaveNetDeviceExample::ReBroadcastPacket, this, SCH1,
                              header.GetSeq (), node_pos, p);
        std::cout << Now ().GetSeconds () << std::endl;
        rebroadcast_check[header.GetPacketId ()][node_pos]++;
      }
  }
  // else
  //   std::cout << "BUG" << std::endl;

  return true;
}

void
WaveNetDeviceExample::InitiateRebroadcastMatrix (uint32_t node_num, uint32_t packet_num)
{
  for (uint32_t i = 0; i < packet_num; i++)
    {
      // Vector to store column elements
      std::vector<uint32_t> v1;

      for (uint32_t j = 0; j < node_num; j++)
        {
          v1.push_back (0);
        }

      // Pushing back above 1D vector
      // to create the 2D vector
      rebroadcast_check.push_back (v1);
    }
  // for (uint32_t i = 0; i < node_num; i++)
  //   {
  //     rebroadcast_check.push_back (0);
  //   }
}
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
             uint32_t (a[2]) << 24 | uint32_t (a[3]) << 16 | uint32_t (a[4]) << 8 |
             uint32_t (a[5]));
}
/*end function*/

void
WaveNetDeviceExample::SendPacketCustomHeader (uint32_t channel, uint64_t seq, uint32_t sender_id,
                                              uint64_t packet_id)
{
  Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
  const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
  const TxInfo txInfo = TxInfo (channel);
  Mac48Address bssWildcard = Mac48Address::GetBroadcast ();

  
  Ptr<Packet> p = Create<Packet> (100);
  MyHeader m_header;
  m_header.SetSeq (sender_id);
  m_header.SetPacketId (packet_id);
  p->AddHeader (m_header);
  // packetSrc.push_back (sender_id);
  std::cout << sender_id << std::endl;
  sender->SendX (p, bssWildcard, WSMP_PROT_NUMBER, txInfo);
}

void
WaveNetDeviceExample::ReBroadcastPacket (uint32_t channel, uint32_t seq, uint32_t sender_id,
                                         Ptr<Packet> pkt)
{
  Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (sender_id));
  const static uint16_t WSMP_PROT_NUMBER = 0x88DC;
  Mac48Address bssWildcard = Mac48Address::GetBroadcast ();
  const TxInfo txInfo = TxInfo (channel);
  sender->SendX (pkt, bssWildcard, WSMP_PROT_NUMBER, txInfo);
}


void
WaveNetDeviceExample::SendPacket ()
{
  CreateWaveNodeCustomMobility ("position.txt");
  // Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice>(devices.Get(12));

  const SchInfo schInfo = SchInfo (SCH1, false, EXTENDED_ALTERNATING);
  // Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, sender, schInfo);

  for (uint32_t i = 0; i != devices.GetN (); ++i)
    {
      Ptr<WaveNetDevice> sender = DynamicCast<WaveNetDevice> (devices.Get (i));
      Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, sender, schInfo);
      Ptr<WaveNetDevice> receiver = DynamicCast<WaveNetDevice> (devices.Get (i));
      Simulator::Schedule (Seconds (0.0), &WaveNetDevice::StartSch, receiver, schInfo);
      // release SCH access
      Simulator::Schedule (Seconds (20.01), &WaveNetDevice::StopSch, sender, SCH1);
      Simulator::Schedule (Seconds (20.01), &WaveNetDevice::StopSch, receiver, SCH1);
    }

  
  for (uint64_t i = 0; i < PACKET_NUM; i++){
    switch(SENDER_NUM){
      case 1:
        Simulator::Schedule (Seconds (0.01 + (0.1 * i)),
                             &WaveNetDeviceExample::SendPacketCustomHeader, this, SCH1, 0, 0, i);
        packetSrc[i] = 0;
        break;
      case 2:
        Simulator::Schedule (Seconds (0.01 + (0.1 * i)),
                             &WaveNetDeviceExample::SendPacketCustomHeader, this, SCH1, 0, 0, i);
        packetSrc[i] = 0;
        Simulator::Schedule (Seconds (0.06+ (0.1 * i)),
                             &WaveNetDeviceExample::SendPacketCustomHeader, this, SCH1, 1, 1,
                             i + PACKET_NUM);
        packetSrc[i + PACKET_NUM] = 1;
        break;
      default:
        break;
      }
  }
    

  Simulator::Stop (Seconds (20.01));
  Simulator::Run ();
  Simulator::Destroy ();

  conn_mat.PrintMatrix ();

  std::cout << std::endl;
  // conn_mat.GetNeighbors ();
  std::cout << std::endl;
  delay_mat.SetNeighborMatrix ();
  delay_mat.PrintMatrix ();
  delay_mat.SetDelayIdxVec (packetSrc, SENDER_NUM);
  std::cout << "The total avg delay index: " << delay_mat.GetTotalAvgDelayIdx () << std::endl;
  
  delay_mat.GetTotalPDR (packetSrc, SENDER_NUM);
  // double min_delay_index = delay_mat.GetMinDelayIndex ();
  // std::cout << min_delay_index << std::endl;
  std::cout << "rebroadcast check:" << std::endl;
  for (uint32_t i = 0; i < rebroadcast_check.size (); i++)
    {
      for (uint32_t j = 0; j < rebroadcast_check[i].size ();j++){
          std::cout << rebroadcast_check[i][j] << " " ;
        }
      std:: cout<<""<<std::endl;
      // std::cout << "Node " << i << " rebroadcast " << rebroadcast_check[i] << " times"
      //           << std::endl;
    }
  for (uint64_t i = 0; i < packetSrc.size ();i++){
      std::cout << "Packet # " << i << "  Src: " << packetSrc[i] << std::endl;
    }
}

void
WaveNetDeviceExample::ScheduleSendPacket (Time time, uint64_t channel, uint64_t sender_id, uint64_t packet_count)
{
  Simulator::Schedule (time, &WaveNetDeviceExample::SendPacketCustomHeader, this, channel, sender_id,sender_id,packet_count);
}


int
main (int argc, char *argv[])
{
  CommandLine cmd;
  uint64_t node_num;
  uint64_t sender_num;
  uint64_t packet_num;
  uint64_t mode;

  cmd.AddValue ("NODE_NUM", "number of node", node_num);
  cmd.AddValue ("SENDER_NUM", "number of sender", sender_num);
  cmd.AddValue ("PACKET_NUM", "number of packet", packet_num);
  cmd.AddValue ("MODE", "mode", mode);

  cmd.Parse (argc, argv);

  WaveNetDeviceExample example(node_num, sender_num, packet_num, mode);
  std::cout << "run WAVE  example case:" << std::endl;
  example.SendPacket ();

  return 0;
}