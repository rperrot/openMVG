// Copyright (c) 2017 Romuald Perrot.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "saddle_detector.hpp"

#include "openMVG/stl/numeric_array.hpp"

#include <algorithm>
#include <array>

namespace openMVG
{
namespace features
{

  /**
   * @brief Initialize detector 
   * @param nb_level Number of pyramid level 
   * @param scaling Scale between two pyramid level 
   * @brief epsilon sensibility for classification af saddle points 
   */
  SaddleDetector::SaddleDetector( const int nb_level, const double scaling, const int epsilon )
      : m_nb_level( nb_level ),
        m_scaling( scaling ),
        m_epsilon( epsilon )
  {
  }

  /**
      * @brief Detect saddle points in the given image 
      * @param ima Image image in which points are extracted
      * @param[out] regions Detected points 
      */
  void SaddleDetector::detect( const image::Image<unsigned char> &ima, std::vector<SIOPointFeature> &regions ) const
  {
    // 1 -> For each pyramid level
    image::Image<unsigned char> cur_slice = ima;

    float scale_x = 1.0f;
    float scale_y = 1.0f;

    double scale = 1.0;

    for ( int id_pyramid = 0; id_pyramid < m_nb_level; ++id_pyramid )
    {
      // 1.1 -> Extract saddle points
      std::vector<PointFeature> tmpPt;
      std::vector<unsigned char> tmpIp;
      alternatingInnerOuter( cur_slice, tmpPt, tmpIp );

      // 1.2 -> Filter saddle points
      std::vector<PointFeature> filteredPts;
      nonMaxSuppression( cur_slice, tmpPt, tmpIp, filteredPts );

      // 1.3 -> Accumulate
      for ( size_t id_pt = 0; id_pt < filteredPts.size(); ++id_pt )
      {
        regions.emplace_back( SIOPointFeature( scale_x * filteredPts[ id_pt ].x(), scale_y * filteredPts[ id_pt ].y(), scale ) );
      }

      // 1.4 -> Final computation
      if ( ( id_pyramid + 1 ) != m_nb_level )
      {
        prepareSlice( cur_slice );
        scale_x = static_cast<float>( ima.Width() ) / static_cast<float>( cur_slice.Width() );
        scale_y = static_cast<float>( ima.Height() ) / static_cast<float>( cur_slice.Height() );
        scale *= m_scaling;
      }
    }
  }

  /**
  * @brief Validate point based on outer ring test 
  * @param arcs List of arcs in the outer ring (label, number of consecutive points)
  * @retval true if outer ring is valid 
  * @retval false if outer ring is not valid  
  */
  static inline bool validOuterRing( const std::vector<std::pair<unsigned char, unsigned char>> &arcs )
  {
    // valid if : exactly 2 consecutives arcs 0 2
    // arcs must be of length 2 to 8
    // if more than 4 arcs, arcs 0 and 2 can be separated by less (or equal) than 2 1 label
    if ( arcs.size() < 4 )
    {
      return false;
    }

    // count number of arcs of each types
    int nb[ 3 ] = {0, 0, 0};
    for ( int i = 0; i < arcs.size(); ++i )
    {
      ++nb[ arcs[ i ].first ];
    }

    // If number of 0 and number of 2 are not 2 -> invalid
    if ( nb[ 0 ] != 2 || nb[ 2 ] != 2 )
    {
      return false;
    }

    // test if arcs have good length
    unsigned char min_length[ 3 ] =
        {
            std::numeric_limits<unsigned char>::max(),
            std::numeric_limits<unsigned char>::max(),
            std::numeric_limits<unsigned char>::max()};
    unsigned char max_length[ 3 ] =
        {
            std::numeric_limits<unsigned char>::lowest(),
            std::numeric_limits<unsigned char>::lowest(),
            std::numeric_limits<unsigned char>::lowest()};
    for ( size_t i = 0; i < arcs.size(); ++i )
    {
      const unsigned char type = arcs[ i ].first;
      const unsigned char len  = arcs[ i ].second;
      min_length[ type ]       = std::min( min_length[ type ], len );
      max_length[ type ]       = std::max( max_length[ type ], len );
    }

    // Ensure length are between 2 and 8 and 1 is less (or equal) 2
    if ( min_length[ 0 ] < 2 || min_length[ 2 ] < 2 ||
         max_length[ 0 ] > 8 || max_length[ 2 ] > 8 ||
         max_length[ 1 ] > 2 )
    {
      return false;
    }

    // finally make sure the arc are alternating
    // Get the first arc index of type 0 or 2 (ie: ignore the first arcs with label 1)
    unsigned char prev_arc;
    int i = 0;
    for ( ; i < arcs.size(); ++i )
    {
      if ( arcs[ i ].first != 1 )
      {
        prev_arc = arcs[ i ].first;
        break;
      }
    }
    ++i;

    // test alternating
    for ( ; i < arcs.size(); ++i )
    {
      if ( arcs[ i ].first == 1 )
      { // it's a neutral arc
        continue;
      }
      if ( arcs[ i ].first == prev_arc )
      {
        // it's the same as the previous one -> invalid
        return false;
      }
      prev_arc = arcs[ i ].first;
    }
    return true;
  }

  /**
  * @brief Compute arcs types and length given labels of each neighbors  
  * @param labels Array of the labels of the neighbors
  * @return Arcs [ie: array of (label type ,nb consecutive) ]
  */
  template <size_t N>
  static inline std::vector<std::pair<unsigned char, unsigned char>> computeArcsLengths( const std::array<unsigned char, N> &labels )
  {
    std::vector<std::pair<unsigned char, unsigned char>> arcs_lengths;
    std::pair<unsigned char, unsigned char> cur_arc( labels[ 0 ], 1 );
    for ( size_t i = 1; i < labels.size(); ++i )
    {
      if ( labels[ i ] == cur_arc.first )
      {
        // it's the same arc
        ++cur_arc.second;
      }
      else
      {
        // another arc
        arcs_lengths.push_back( cur_arc );
        cur_arc = std::make_pair( labels[ i ], 1 );
      }
    }

    // Compute last arc (is it the same type as the first one ?)
    if ( arcs_lengths.size() > 0 )
    {
      if ( arcs_lengths[ 0 ].first == cur_arc.first )
      {
        // same arc
        arcs_lengths[ 0 ].second += cur_arc.second;
      }
      else
      {
        // another arc type
        arcs_lengths.push_back( cur_arc );
      }
    }
    else
    {
      // nothing, this is a unique arc
      arcs_lengths.push_back( cur_arc );
    }
    return arcs_lengths;
  }

  /**
    * @brief compute points that pass alternating inner test and outer tests 
    * @param ima Image image in which points are extracted
    * @param[out] pts The points position 
    * @param[out] Ips The interpolated feature intensity (at the saddle point)
    */
  void SaddleDetector::alternatingInnerOuter( const image::Image<unsigned char> &ima,
                                              std::vector<PointFeature> &pts,
                                              std::vector<unsigned char> &Ips ) const
  {
    // 6 -> 3 for neighbor + 3 for neighbors of the neighbors
    for ( int id_row = 6; id_row < ima.Height() - 6; ++id_row )
    {
      for ( int id_col = 6; id_col < ima.Width() - 6; ++id_col )
      {
        // Get a local vector of queried points
        /*
        | 0 | . | 1 | . | 2 |
        | . | . | . | . | . |
        | 3 | . | p | . | 4 |
        | . | . | . | . | . | 
        | 5 | . | 6 | . | 7 | 
         */
        const std::array<unsigned char, 8> data = {ima( id_row - 2, id_col - 2 ), ima( id_row - 2, id_col ), ima( id_row - 2, id_col + 2 ),
                                                   ima( id_row, id_col - 2 ), ima( id_row, id_col + 2 ),
                                                   ima( id_row + 2, id_col - 2 ), ima( id_row + 2, id_col ), ima( id_row + 2, id_col + 2 )};

        // Detect +
        const unsigned char min_x = std::min( data[ 3 ], data[ 4 ] );
        const unsigned char max_x = std::max( data[ 3 ], data[ 4 ] );
        const unsigned char min_y = std::min( data[ 1 ], data[ 6 ] );
        const unsigned char max_y = std::max( data[ 1 ], data[ 6 ] );

        const bool cross_valid = min_x > max_y || min_y > max_x;

        // Detect x
        const unsigned char min_xy = std::min( data[ 0 ], data[ 7 ] );
        const unsigned char max_xy = std::max( data[ 0 ], data[ 7 ] );
        const unsigned char min_yx = std::min( data[ 2 ], data[ 5 ] );
        const unsigned char max_yx = std::max( data[ 2 ], data[ 5 ] );

        const bool diag_valid = min_xy > max_yx || min_yx > max_xy;

        // We've found a point in the inner ring
        if ( cross_valid || diag_valid )
        {
          // 1 - > Compute Intensity of the central pixel
          int Ip;
          if ( cross_valid && diag_valid )
          {
            // Both -> Use the 8 values
            Ip = stl::numeric_array<unsigned char, 8>::median( data.begin(), data.end() );
          }
          else if ( cross_valid )
          {
            // Cross
            const std::array<unsigned char, 4> tmp = {data[ 1 ], data[ 3 ], data[ 4 ], data[ 6 ]};
            Ip = stl::numeric_array<unsigned char, 4>::median( data.begin(), data.end() );
          }
          else
          {
            // Diag
            const std::array<unsigned char, 4ul> tmp = {data[ 0 ], data[ 2 ], data[ 5 ], data[ 7 ]};
            Ip = stl::numeric_array<unsigned char, 4ul>::median( data.begin(), data.end() );
          }

          // 2 -> Compute Labels of each neigborings pixels
          const std::array<int, 16> b =
              {
                  // b1 b2
                  ima( id_row + 3, id_col ), ima( id_row + 3, id_col + 1 ),
                  // b3
                  data[ 7 ],
                  // b4 b5 b6
                  ima( id_row + 1, id_col + 3 ), ima( id_row, id_col + 3 ), ima( id_row - 1, id_col + 3 ),
                  // b7
                  data[ 2 ],
                  // b8 b9 b10
                  ima( id_row - 3, id_col + 1 ), ima( id_row - 3, id_col ), ima( id_row - 3, id_col - 1 ),
                  // b11
                  data[ 0 ],
                  // b12 b13 b14
                  ima( id_row - 1, id_col - 3 ), ima( id_row, id_col - 3 ), ima( id_row + 1, id_col - 3 ),
                  // b15
                  data[ 5 ],
                  // b16
                  ima( id_row + 3, id_col - 1 )};

          // Compute labels for each pixels of the outer ring
          std::array<unsigned char, 16> labels;
          for ( int i = 0; i < 16; ++i )
          {
            if ( b[ i ] < Ip - m_epsilon )
            {
              labels[ i ] = 0;
            }
            else if ( Ip - m_epsilon <= b[ i ] && b[ i ] <= Ip + m_epsilon )
            {
              labels[ i ] = 1;
            }
            else if ( b[ i ] > Ip + m_epsilon )
            {
              labels[ i ] = 2;
            }
          }

          // 3 - Test if outer ring is valid
          // Compute length of arcs
          const std::vector<std::pair<unsigned char, unsigned char>> arcs_lengths = computeArcsLengths( labels );

          // Test if outer ring is valid
          if ( validOuterRing( arcs_lengths ) )
          {
            // add a point
            pts.emplace_back( PointFeature( id_col, id_row ) );
            Ips.emplace_back( Ip );
          }
        }
      }
    }
  }

  /**
    * @brief Compute non max suppression and final point localisation 
    * @param ima Image 
    * @param putative Putative points to filter 
    * @param putativeIps Putative points intensity to filter 
    * @param[out] filteredPts Final points after filtering and position assignment 
    */
  void SaddleDetector::nonMaxSuppression( const image::Image<unsigned char> &ima,
                                          const std::vector<PointFeature> &putatives,
                                          const std::vector<unsigned char> &putativeIps,
                                          std::vector<PointFeature> &filteredPts ) const
  {
    bool non_max_suppr = true;

    // Neighboring coordinates
    // Y - X
    static const int n_pos[ 16 ][ 2 ] =
        {
            // b1
            {3, 0},
            {3, 1},
            {2, 2},
            {1, 3},
            {0, 3},
            {-1, 3},
            {-2, 2},
            // b8
            {-3, 1},
            {-3, 0},
            {-3, -1},
            {-2, -2},
            {-1, -3},
            {0, -3},
            {1, -3},
            {2, -2},
            {3, -1},
        };

    // 0 - > Fill image with precomputed Ips
    image::Image<unsigned int>
        response_map( ima.Width(), ima.Height() , true , 0 );
    for ( int id_pt = 0; id_pt < putatives.size(); ++id_pt )
    {
      const int id_row = putatives[ id_pt ].y();
      const int id_col = putatives[ id_pt ].x();

      int resp = 0;
      for ( int id_n = 0; id_n < 16; ++id_n )
      {
        const int b = ima( id_row + n_pos[ id_n ][ 0 ], id_col + n_pos[ id_n ][ 1 ] );
        resp += std::abs( static_cast<int>( putativeIps[ id_pt ] ) - b );
      }

      response_map( id_row, id_col ) = resp;
    }

    // Neighboring in 3x3
    // Y - X
    /*
     * Ording is like this : 
     * 
     * 0 1 2
     * 7   3
     * 6 5 4
     * 
     */
    static const int n_pos_9[ 8 ][ 2 ] =
        {
            {-1, -1},
            {-1, 0},
            {-1, 1},

            {0, 1},
            {1, 1},
            {1, 0},

            {1, -1},
            {0, -1}};

    for ( size_t id_pt = 0; id_pt < putatives.size(); ++id_pt )
    {
      const int id_row = putatives[ id_pt ].y();
      const int id_col = putatives[ id_pt ].x();

      // 1 - perform non maximum suppression
      if ( non_max_suppr )
      {
        bool is_max  = true;
        const int rn = response_map( id_row, id_col );
        for ( int id_n = 0; id_n < 8; ++id_n )
        {
          if ( response_map( id_row + n_pos_9[ id_n ][ 0 ], id_col + n_pos_9[ id_n ][ 1 ] ) > rn )
          {
            is_max = false;
            break;
          }
        }
        if ( !is_max )
        {
          continue;
        }
      }

      // 2 - Compute final position based
      // 0 - Compute 3x3 neighboring response
      float dx = 0.0f;
      float dy = 0.0f;
      int n_resp9[ 8 ];
      int sum_resp = 0;
      for ( int id_n = 0; id_n < 8; ++id_n )
      {
        const int n_id_row = id_row + n_pos_9[ id_n ][ 0 ];
        const int n_id_col = id_col + n_pos_9[ id_n ][ 1 ];

        const int r_n = response_map( n_id_row, n_id_col );
        sum_resp += r_n;

        dx += n_pos_9[ id_n ][ 1 ] * r_n;
        dy += n_pos_9[ id_n ][ 0 ] * r_n;
      }
      if ( sum_resp != 0 )
      {
        dx /= static_cast<float>( sum_resp );
        dy /= static_cast<float>( sum_resp );
      }

      filteredPts.emplace_back( PointFeature( static_cast<float>( id_col ) + dx, static_cast<float>( id_row ) + dy ) );
    }
  }

  /**
    * @brief Compute new pyramid level from current one 
    * @param[in,out] slice Current and output slice 
    */
  void SaddleDetector::prepareSlice( image::Image<unsigned char> &slice ) const
  {
    image::Image<unsigned char> tmp;
    ImageRescale( slice, tmp, 1.0 / m_scaling );
    slice = tmp;
  }

} // namespace features
} // namespace openMVG
