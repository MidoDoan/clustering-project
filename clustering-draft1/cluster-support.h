#ifndef CLUSTER_SUPPORT_H
#define CLUSTER_SUPPORT_H
#include "ns3/vector.h"
#include<map>
// #include <stdint.h>
// #include <vector>
namespace ns3 {
  class ClusterSupport {
    public:
      virtual ~ClusterSupport ();

      enum NodeDegree { UN = 0, CH, CM, RSU};
      enum DataInfo { NOTIFICATION, EMERGENCY_EVENT = 0};

      struct BeaconInfo{
        uint64_t nodeId;
        double posX;
        double posY;
        double posZ;
        double velX;
        double velY;
        double velZ;
      };
      struct NodeInfo
      {
        uint64_t clusterId;
        uint64_t nodeId;
        double posX;
        double posY;
        double posZ;
        double velX;
        double velY;
        double velZ;
        
        NodeDegree degree;
      };
      struct ClusterInfo {
        uint64_t clusteId;
        std::map<uint64_t, NodeInfo> clusterMem;
      };
  };

} // namespace ns3
#endif
