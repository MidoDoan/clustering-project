#ifndef CLUSTER_POS_H
#define CLUSTER_POS_H
#include <string>
#include<vector>
#include "ns3/node-container.h"

namespace ns3 {
  class Position{
    private:
      uint64_t nodeNum;
      std::vector<double> posSet;

    public:
      Position (std::string file_path);
      NodeContainer GetContainer (uint64_t laneNum, double laneWid, int64_t m_nodeNum);
      uint64_t GetNodeNum ();
  };
  } // namespace ns3

#endif