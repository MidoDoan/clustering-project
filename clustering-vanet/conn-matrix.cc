#include "conn-matrix.h"
#include <iostream>
#include <numeric>
#include <bits/stdc++.h>
namespace ns3 {
void
ConnectivityMatrix::InitiateMatrix (uint64_t row_num, uint64_t col_num)
{

  for (uint64_t i = 0; i < row_num; i++)
    {
      // Vector to store column elements
      std::vector<double> v1;

      for (uint64_t j = 0; j < col_num; j++)
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
  // for (uint64_t row = 0; row < matrix.size (); row++)
  //   {
  //     // for (uint64_t col = 0; col < matrix[0].size (); col++)
  //     //   {
  //     //     std::cout << matrix[row][col] << " ";
  //     //   }
  //     // std::cout << std::endl;
  //     Print1D (matrix[row]);
  //   }
  Print2D (matrix);
}

void
ConnectivityMatrix::InsertEntry (uint64_t node_row, uint64_t node_col, double data)
{
  matrix[node_row][node_col] = data;
}

void
ConnectivityMatrix::GetNeighbors ()
{
  for (uint64_t j = 0; j < matrix.size (); j++)
    {
      std::vector<uint64_t> row;
      for (uint64_t i = 0; i < matrix.size (); i++)
        {
          if (matrix[j][i] != 0)
            {
              row.push_back (i);
            }
        }
      neighbor_matrix.push_back (row);
    }
  for (uint64_t j = 0; j < neighbor_matrix.size (); j++)
    {
      std::cout << "Neighbor[" << j << "]: ";
      for (uint64_t i = 0; i < neighbor_matrix[j].size (); i++)
        {
          std::cout << neighbor_matrix[j][i] << " ";
        }
      std::cout << std::endl;
    }
}

void
ConnectivityMatrix::GetReceiveNodes (uint64_t sender_id)
{
  uint64_t count = 0;
  for (uint64_t i = 0; i < matrix.size (); i++)
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
ConnectivityMatrix::GetElement (uint64_t node_row, uint64_t node_col)
{
  return matrix[node_row][node_col];
}

void
ConnectivityMatrix::GetDelayIndexes ()
{
  for (uint64_t i = 0; i < matrix.size (); i++)
    {
      double delay_index =
          std::accumulate (matrix[i].begin (), matrix[i].end (), 0.0) / matrix[i].size ();
      delay_index_vec.push_back (delay_index);
      std::cout << "DelayIndex[" << i << "]: " << delay_index << std::endl;
    }
}

uint64_t
ConnectivityMatrix::GetNeighborNum (uint64_t node_id)
{
  std::vector<uint64_t> row;
  for (uint64_t i = 0; i < matrix.size (); i++)
    {
      if (matrix[node_id][i] != 0)
        {
          row.push_back (i);
        }
    }
  return row.size ();
}

double
ConnectivityMatrix::GetMinDelayIndex ()
{
  return min_element (delay_index_vec.begin (), delay_index_vec.end ()) - delay_index_vec.begin ();
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
