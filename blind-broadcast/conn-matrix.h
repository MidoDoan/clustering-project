#ifndef CONN_MATRIX_H
#define CONN_MATRIX_H
#include "ns3/vector.h"
#include <map>
// #include <stdint.h>
#include <vector>
namespace ns3 {

class ConnectivityMatrix
{
private:
  std::vector<std::vector<double>> matrix;
  std::vector<std::vector<uint32_t>> neighbor_matrix;
  std::vector<double> delay_index_vec;
  std::vector<std::vector<double>> delay_index_vector;
  std::vector<double> delay_idx;
  uint32_t row_num;
  uint32_t col_num;

public:
  void PrintMatrix ();
  void InitiateMatrix (uint32_t row_num, uint32_t col_num);
  void GetReceiveNodeNum (uint32_t sender_id);
  void GetNeighbors ();
  uint32_t GetNeighborNum (uint32_t node_id);
  void GetTotalPDR (std::vector<uint64_t> packet_src, uint32_t src_num);
  void InsertEntry (uint32_t node_row, uint32_t node_col, double data);
  double GetElement (uint32_t node_row, uint32_t node_col);
  void GetDelayIndexes ();
  double GetMinDelayIndex ();
  double GetTotalAvgDelayIdx ();
  void Print1D (std::vector<double> vector);
  void Print2D (std::vector<std::vector<double>> vector);
  void SetNeighborMatrix ();
  void SetDelayIdxVec (std::vector<uint64_t> packet_src, uint32_t sender_num);
};
} // namespace ns3
#endif
