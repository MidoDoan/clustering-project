#include "cluster-pos.h"
#include <fstream>
#include "ns3/log.h"
#include "ns3/mobility-helper.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("Cluster Positionn");

Position::Position (std::string file_path)
{
  std::ifstream file (file_path);
  std::string str;
  nodeNum = 0;
  while (std::getline (file, str)){
    nodeNum++;
    double pos = std::stod (str);
    posSet.push_back (pos);
  }
  /*Check pos list*/
  // for (std::vector<double>::iterator it = posSet.begin ();
  //      (it != posSet.end ()); ++it){
  //     std::cout << *it << " ";
  //   }
  // std::cout<<" "<<std::endl;
}

NodeContainer
Position::GetContainer (uint64_t laneNum, double laneWid, int64_t m_nodeNum){
  NodeContainer nodes = NodeContainer();
  nodes.Create (nodeNum);
  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  for (std::vector<double>::iterator it = posSet.begin (); (it != posSet.end())||(it-posSet.begin()) < m_nodeNum;++it)
  {
    positionAlloc->Add (Vector (*it, ((it-posSet.begin()) % laneNum) * laneWid, 0.0));
  }
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (nodes);
  return nodes;
}

uint64_t 
Position:: GetNodeNum(){
  return nodeNum;
}
}
