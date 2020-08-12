#include "conn-matrix.h"
#include <iostream>
#include <numeric>
#include <bits/stdc++.h>
namespace ns3 {
void
ConnectivityMatrix::InitiateMatrix (
    uint32_t row_num, uint32_t col_num) // row_num= packets of 1 node send * number of nodes
{

  for (uint32_t i = 0; i < row_num; i++)
    {
      // Vector to store column elements
      std::vector<double> v1;

      for (uint32_t j = 0; j < col_num; j++)
        {
          v1.push_back (0);
        }

      // Pushing back above 1D vector
      // to create the 2D vector
      matrix.push_back (v1);
    }
}

void
ConnectivityMatrix::PrintMatrix ()
{
  // for (uint32_t row = 0; row < matrix.size (); row++)
  //   {
  //     // for (uint32_t col = 0; col < matrix[0].size (); col++)
  //     //   {
  //     //     std::cout << matrix[row][col] << " ";
  //     //   }
  //     // std::cout << std::endl;
  //     Print1D (matrix[row]);
  //   }
  Print2D (matrix);
}

void
ConnectivityMatrix::InsertEntry (uint32_t node_row, uint32_t node_col, double data)
{
  matrix[node_row][node_col] = data;
}

void
ConnectivityMatrix::SetNeighborMatrix ()
{
  for (uint32_t j = 0; j < matrix.size (); j++)
    {
      std::vector<uint32_t> row;
      for (uint32_t i = 0; i < matrix[j].size (); i++)
        {
          if (matrix[j][i] != 0)
            {
              row.push_back (i);
            }
        }
      neighbor_matrix.push_back (row);
    }
}

void
ConnectivityMatrix::GetNeighbors ()
{
  for (uint32_t j = 0; j < neighbor_matrix.size (); j++)
    {
      std::cout << "Neighbor[" << j << "]: ";
      for (uint32_t i = 0; i < neighbor_matrix[j].size (); i++)
        {
          std::cout << neighbor_matrix[j][i] << " ";
        }
      std::cout << std::endl;
    }
}

void
ConnectivityMatrix::GetReceiveNodeNum (uint32_t sender_id)
{
  uint32_t count = 0;
  for (uint32_t i = 0; i < matrix[0].size (); i++)
    {
      if (matrix[sender_id][i] != 0)
        {
          std::cout << "The receiver node is: " << i << std::endl;
          count++;
        }
    }
  std::cout << "# of receiver nodes: " << count << std::endl;
  // return count;
}

double
ConnectivityMatrix::GetElement (uint32_t node_row, uint32_t node_col)
{
  return matrix[node_row][node_col];
}

// void
// ConnectivityMatrix::GetDelayIndexes ()
// {
//   for (uint32_t i = 0; i < matrix.size (); i++) //matrix ==delay matrix
//     {
//       // double delay_index =
//       //     std::accumulate (matrix[i].begin (), matrix[i].end (), 0.0) / matrix[i].size ();
//       // delay_index_vec.push_back (delay_index);
//       // std::cout << "DelayIndex[" << i << "]: " << delay_index << std::endl;
//       double delay_index;
//       if (GetNeighborNum (i) == 0)
//         {
//           delay_index = 0;
//         }
//       else
//         {
//           delay_index =
//               std::accumulate (matrix[i].begin (), matrix[i].end (), 0.0) / GetNeighborNum (i);
//         }
//       delay_index_vec.push_back (delay_index);
//       std::cout << "DelayIndex[" << i << "]: " << delay_index << std::endl;
//     }
// }

//TEST THIS FUNCTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void
ConnectivityMatrix::SetDelayIdxVec (std::vector<uint64_t> packet_src, uint32_t sender_num)
{
  for (uint32_t i = 0; i < matrix.size (); i++) //matrix ==delay matrix
    {
      // double delay_index =
      //     std::accumulate (matrix[i].begin (), matrix[i].end (), 0.0) / matrix[i].size ();
      // delay_index_vec.push_back (delay_index);
      // std::cout << "DelayIndex[" << i << "]: " << delay_index << std::endl;
      double delay_index;
      if (GetNeighborNum (i) == 0)
        {
          delay_index = 0;
        }
      else
        {
          delay_index =
              std::accumulate (matrix[i].begin (), matrix[i].end (), 0.0) / GetNeighborNum (i);
        }
      std::cout << "DelayIdxPacket [" << i << "]: " << delay_index << std::endl;
      delay_index_vector.resize (matrix[0].size());
      delay_index_vector[packet_src[i]].push_back (delay_index);
      // std::cout << "DelayIndex[" << i << "]: " << delay_index << std::endl;
    }
  for (uint32_t i = 0; i < delay_index_vector.size (); i++)
    {
      double sum = 0;
      int size;
      size = delay_index_vector[i].size ();
      for (uint32_t j = 0; j < delay_index_vector[i].size (); j++)
        {
          if (delay_index_vector[i][j] != 0)
            {
              sum += delay_index_vector[i][j];
            }
          else
            {
              size = size - 1;
            }
        }

      double avg_delay_idx;
      if (size <= 0)
        {
          avg_delay_idx = 0;
        }
      else
        avg_delay_idx = sum / size;
      delay_idx.push_back (avg_delay_idx);
      std::cout << "DelayIdxNode [" << i << "]: " << avg_delay_idx << std::endl;
    }
}

// double
// ConnectivityMatrix::GetTotalAvgDelayIdx ()
// {
//   int size = delay_index_vec.size ();// CHECK1111111111
//   double sum = 0;
//   for (uint32_t i = 0; i < delay_index_vec.size (); i++)
//     {
//       if (delay_index_vec[i] != 0)
//         {
//           sum += delay_index_vec[i];
//         }
//       else
//         {
//           size--;
//         }
//     }
//   return sum / size;
//   // while (delay_index_vec[i] != 0)
//   //   return std::accumulate (delay_index_vec.begin (), delay_index_vec.end (), 0.0) /
//   //          delay_index_vec.size ();
// }
double
ConnectivityMatrix::GetTotalAvgDelayIdx ()
{
  int size = delay_idx.size (); // CHECK1111111111
  double sum = 0;
  for (uint32_t i = 0; i < delay_idx.size (); i++)
    {
      if (delay_idx[i] != 0)
        {
          sum += delay_idx[i];
        }
      else
        {
          size--;
        }
    }
  return sum / size;
  // while (delay_index_vec[i] != 0)
  //   return std::accumulate (delay_index_vec.begin (), delay_index_vec.end (), 0.0) /
  //          delay_index_vec.size ();
}

uint32_t
ConnectivityMatrix::GetNeighborNum (uint32_t node_id)
{
  // std::vector<uint32_t> row;
  // for (uint32_t i = 0; i < matrix.size (); i++)
  //   {
  //     if (matrix[node_id][i] != 0)
  //       {
  //         row.push_back (i);
  //       }
  //   }
    return neighbor_matrix[node_id].size ();
}

// double
// ConnectivityMatrix::GetMinDelayIndex ()
// {
//   return min_element (delay_index_vec.begin (), delay_index_vec.end ()) - delay_index_vec.begin ();
// }

void ConnectivityMatrix::GetTotalPDR (std::vector<uint64_t> packet_src, uint32_t src_num){
  std::vector<std::vector<double>> nodePDR; // contain the PDR of packets that node sent
  std::vector<double> avgNodePDR; // contain the avg PDR of nodes
  nodePDR.resize (matrix[0].size());
  for (uint32_t i = 0; i < matrix.size (); i++)
    {
      uint32_t recv_num = GetNeighborNum (i);
      double individualPDR = (double)recv_num / (matrix[i].size ()-1);
      nodePDR[packet_src[i]].push_back (individualPDR);
    }
  for (uint32_t i = 0; i < nodePDR.size ();i++){
    if(nodePDR[i].size()!=0){
        avgNodePDR.push_back (std::accumulate (nodePDR[i].begin (), nodePDR[i].end (), 0.0) /
                              nodePDR[i].size ());
    }
      
    }
  for (uint32_t i = 0; i < avgNodePDR.size (); i++){
      std::cout << "The avg PDR of node send: " << i << ": " << avgNodePDR[i] << std::endl;
    }
  double totalAVGPDR = std::accumulate (avgNodePDR.begin (),avgNodePDR.end (), 0.0) /
                            avgNodePDR.size ();
  std::cout << "The total avg PDR: " << totalAVGPDR << std::endl;
}
void
ConnectivityMatrix::Print1D (std::vector<double> vector)
{
  for (std::vector<double>::iterator it = vector.begin (); it != vector.end (); ++it)
    {
      std::cout << *it << " ";
    }
  // std::cout<<std::endl;
}

void
ConnectivityMatrix::Print2D (std::vector<std::vector<double>> vector)
{
  for (std::vector<std::vector<double>>::iterator it = vector.begin (); it != vector.end (); ++it)
    {
      Print1D (vector[it - vector.begin ()]);
      std::cout << std::endl;
    }
  std::cout << std::endl;
}

// bool
// ConnectivityMatrix::IsNeighbor (uint64_t node1, uint64_t node2)
// {
//   for (uint64_t i = 0; i < neighbor_matrix[node1].size (); i++)
//     {
//       if (neighbor_matrix[node1][i] == node2)
//         {
//           return true;
//         }
//       else
//         return false;
//     }
// }
} // namespace ns3
