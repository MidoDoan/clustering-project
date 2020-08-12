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
  std::vector<std::vector<uint64_t>> neighbor_matrix;
  std::vector<double> delay_index_vec;
  uint64_t row_num;
  uint64_t col_num;

public:
  void PrintMatrix ();
  void InitiateMatrix (uint64_t row_num, uint64_t col_num);
  void GetReceiveNodes (uint64_t sender_id);
  void GetNeighbors ();
  uint64_t GetNeighborNum (uint64_t node_id);
  void InsertEntry (uint64_t node_row, uint64_t node_col, double data);
  double GetElement (uint64_t node_row, uint64_t node_col);
  void GetDelayIndexes ();
  double GetMinDelayIndex ();
  void Print1D (std::vector<double> vector);
  void Print2D (std::vector<std::vector<double>> vector);
  // bool IsNeighbor (uint64_t node1, uint64_t node2);
};
} // namespace ns3
#endif
