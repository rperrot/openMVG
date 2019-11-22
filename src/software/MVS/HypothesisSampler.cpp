#include "HypothesisSampler.hpp"

namespace MVS
{

static inline void DetermineNeighborBySampling( const openMVG::image::Image<double>& costs,
                                                const int                            id_row,
                                                const int                            id_col,
                                                const std::vector<std::vector<int>>& putative,
                                                std::vector<int>&                    res )
{
  int    best_id   = -1;
  double best_cost = std::numeric_limits<double>::max();
  for ( int id = 0; id < putative.size(); ++id )
  {
    const int y = id_row + putative[ id ][ 1 ];
    const int x = id_col + putative[ id ][ 0 ];

    if ( costs.Contains( y, x ) )
    {
      const double cost = costs( y, x );
      if ( cost < best_cost )
      {
        best_cost = cost;
        best_id   = id;
      }
    }
  }

  if ( best_id != -1 )
  {
    res[ 0 ] = putative[ best_id ][ 0 ];
    res[ 1 ] = putative[ best_id ][ 1 ];
  }
  else
  {
    res[ 0 ] = std::numeric_limits<int>::max();
    res[ 1 ] = std::numeric_limits<int>::max();
  }
}

/**
 * @brief Get the best neighbors to use for sampling 
 * 
 * @param map 
 * @param id_row 
 * @param id_col 
 * @return std::vector<std::vector<int>> 
 */
static inline void DetermineNeighborBySampling( const openMVG::image::Image<double>& cost,
                                                const int                            id_row,
                                                const int                            id_col,
                                                std::vector<std::vector<int>>&       res )
{
  // Checkboard close north - asymetric
  static const std::vector<std::vector<int>> neigh_idx_asymetric_close_north =
      {
          {0, -1},
          {-1, -2},
          {1, -2},
          {-2, -3},
          {2, -3},
          {-3, -4},
          {3, -4}};
  // Checkboard close south - asymetric
  static const std::vector<std::vector<int>> neigh_idx_asymetric_close_south =
      {
          {0, 1},
          {-1, 2},
          {1, 2},
          {-2, 3},
          {2, 3},
          {-3, 4},
          {3, 4}};
  // Checkboard close west - asymetric
  static const std::vector<std::vector<int>> neigh_idx_asymetric_close_west =
      {
          {-1, 0},
          {-2, -1},
          {-2, 1},
          {-3, -2},
          {-3, 2},
          {-4, -3},
          {-4, 3}};
  // Checkboard close est - asymetric
  static const std::vector<std::vector<int>> neigh_idx_asymetric_close_east =
      {
          {1, 0},
          {2, -1},
          {2, 1},
          {3, -2},
          {3, 2},
          {4, -3},
          {4, 3}};
  static const std::vector<std::vector<int>> neigh_idx_asymetric_far_north =
      {
          {0, -3},
          {0, -5},
          {0, -7},
          {0, -9},
          {0, -11},
          {0, -13},
          {0, -15},
          {0, -17},
          {0, -19},
          {0, -21},
          {0, -23}};
  static const std::vector<std::vector<int>> neigh_idx_asymetric_far_south =
      {
          {0, 3},
          {0, 5},
          {0, 7},
          {0, 9},
          {0, 11},
          {0, 13},
          {0, 15},
          {0, 17},
          {0, 19},
          {0, 21},
          {0, 23}};
  static const std::vector<std::vector<int>> neigh_idx_asymetric_far_west =
      {
          {-3, 0},
          {-5, 0},
          {-7, 0},
          {-9, 0},
          {-11, 0},
          {-13, 0},
          {-15, 0},
          {-17, 0},
          {-19, 0},
          {-21, 0},
          {-23, 0}};
  static const std::vector<std::vector<int>> neigh_idx_asymetric_far_east =
      {
          {3, 0},
          {5, 0},
          {7, 0},
          {9, 0},
          {11, 0},
          {13, 0},
          {15, 0},
          {17, 0},
          {19, 0},
          {21, 0},
          {23, 0}};

  DetermineNeighborBySampling( cost, id_row, id_col, neigh_idx_asymetric_close_north, res[ 0 ] );
  DetermineNeighborBySampling( cost, id_row, id_col, neigh_idx_asymetric_close_south, res[ 1 ] );
  DetermineNeighborBySampling( cost, id_row, id_col, neigh_idx_asymetric_close_west, res[ 2 ] );
  DetermineNeighborBySampling( cost, id_row, id_col, neigh_idx_asymetric_close_east, res[ 3 ] );
  DetermineNeighborBySampling( cost, id_row, id_col, neigh_idx_asymetric_far_north, res[ 4 ] );
  DetermineNeighborBySampling( cost, id_row, id_col, neigh_idx_asymetric_far_south, res[ 5 ] );
  DetermineNeighborBySampling( cost, id_row, id_col, neigh_idx_asymetric_far_west, res[ 6 ] );
  DetermineNeighborBySampling( cost, id_row, id_col, neigh_idx_asymetric_far_east, res[ 7 ] );
}

std::pair<std::vector<openMVG::Vec4>, std::vector<openMVG::Vec2i>> getPropagationHypothesis( const openMVG::image::Image<openMVG::Vec4>& planes,
                                                                                             const openMVG::image::Image<double>&        cost,
                                                                                             const int                                   id_row,
                                                                                             const int                                   id_col,
                                                                                             const DepthMapComputationParameters&        params )
{
  // (x,y) For full preset
  /*
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   | X |   | X |   | X | O | X |   | X |   | X |
   *   |   |   |   | X |   | X |   | X |   |   |   |
   *   |   |   |   |   | X |   | X |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   */
  static const std::vector<std::vector<int>> neighs_idx_full = //[20][2] =
      {
          {0, -5},

          {0, -3},

          {-1, -2},
          {1, -2},

          {-2, -1},
          {0, -1},
          {2, -1},

          {-5, 0},
          {-3, 0},
          {-1, 0},
          {1, 0},
          {3, 0},
          {5, 0},

          {-2, 1},
          {0, 1},
          {2, 1},

          {-1, 2},
          {1, 2},

          {0, 3},

          {0, 5}};

  // (x,y) For speed preset
  /*
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   | X |   |   |   | X | O | X |   |   |   | X |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   |   |   |   |   |   |   |
   *   |   |   |   |   |   | X |   |   |   |   |   |
   */
  static const std::vector<std::vector<int>> neighs_idx_speed = //[8][2] =
      {
          {0, -5},
          {0, -1},
          {-5, 0},
          {-1, 0},
          {1, 0},
          {5, 0},
          {0, 1},
          {0, 5}};

  int                           nb_neigh;
  std::vector<std::vector<int>> neighs_idx;
  switch ( params.propagationScheme() )
  {
    case PROPAGATION_SCHEME_FULL:
    {
      nb_neigh   = 20;
      neighs_idx = neighs_idx_full;
      break;
    }
    case PROPAGATION_SCHEME_SPEED:
    {
      nb_neigh   = 8;
      neighs_idx = neighs_idx_speed;
      break;
    }
    case PROPAGATION_SCHEME_ASYMETRIC:
    {
      nb_neigh = 8; // 1 per region
      neighs_idx.resize( 8 );
      for ( int i = 0; i < 8; ++i )
      {
        neighs_idx[ i ].resize( 2 );
      }

      DetermineNeighborBySampling( cost, id_row, id_col, neighs_idx );
      break;
    }
    default:
    {
      nb_neigh   = 20;
      neighs_idx = neighs_idx_full;
    }
  }

  std::vector<openMVG::Vec4>  hyp_plane;
  std::vector<openMVG::Vec2i> hyp_position;

  // Build hypothesis vector
  for ( int id_n = 0; id_n < nb_neigh; ++id_n )
  {
    const int dx = neighs_idx[ id_n ][ 0 ];
    const int dy = neighs_idx[ id_n ][ 1 ];
    if ( ( dx == std::numeric_limits<int>::max() ) && ( dy == std::numeric_limits<int>::max() ) )
      continue;

    const int x = id_col + dx;
    const int y = id_row + dy;

    if ( cost.Contains( y, x ) )
    {
      hyp_plane.emplace_back( planes( y, x ) );
      hyp_position.emplace_back( openMVG::Vec2i( x, y ) );
    }
  }

  return std::make_pair( hyp_plane, hyp_position );
}

} // namespace MVS
