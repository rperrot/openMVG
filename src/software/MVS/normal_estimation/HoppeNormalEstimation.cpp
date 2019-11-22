#include "normal_estimation/HoppeNormalEstimation.hpp"

namespace MVS
{
/**
   * @brief Construct a new Hoppe Normal Estimation object
   * 
   * @param size Size of the neighboring for normal estimation 
   */
HoppeNormalEstimation::HoppeNormalEstimation( const int size )
    : m_size( size )
{
}

/**
     * @brief Main function used to compute normal of a depth map
     * 
     * @param cam     Camera associated with the depth map 
     * @param map     Depth map to use
     * @param scale   Scale of the computation 
     */
void HoppeNormalEstimation::computeNormals( const Camera& cam, DepthMap& map, const int scale )
{
  const size_t half_size = m_size / 2;

  // 1. Unproject points
  Eigen::Matrix<openMVG::Vec3, Eigen::Dynamic, Eigen::Dynamic> pts;
  pts.resize( map.height(), map.width() );
  for ( size_t id_row = half_size; id_row < map.height() - half_size; ++id_row )
  {
    for ( size_t id_col = half_size; id_col < map.width() - half_size; ++id_col )
    {
      pts( id_row, id_col ) = cam.unProject( id_col, id_row, map.depth( id_row, id_col ), scale );
    }
  }

  // 2. Perform normal estimation
  for ( size_t id_row = half_size; id_row < map.height() - half_size; ++id_row )
  {
    for ( size_t id_col = half_size; id_col < map.width() - half_size; ++id_col )
    {
      // 2.1 Compute centroid of the patch around point (id_col,id_row):
      openMVG::Vec3 c     = openMVG::Vec3::Zero();
      size_t        nb_ok = 0; // Number of valid neighbors
      for ( size_t y = id_row - half_size; y <= id_row + half_size; ++y )
      {
        for ( size_t x = id_col - half_size; x <= id_col + half_size; ++x )
        {
          if ( map.depth( y, x ) > 0.0 ) // Check if neighbors point is valid before using it.
          {
            c += pts( y, x );
            ++nb_ok;
          }
        }
      }
      if ( nb_ok == 0 )
        continue;

      c *= ( 1.0 / static_cast<double>( nb_ok ) );

      // 2.2 Assemble covariance matrix
      // cov( u , v ) = p[u] . p[v]
      // with: p = pts(i) - c , for all p(i) in neighboring.
      openMVG::Mat3 cov = openMVG::Mat3::Zero();

      double accum[ 9 ] = {0};

      for ( size_t y = id_row - half_size; y <= id_row + half_size; ++y )
      {
        for ( size_t x = id_col - half_size; x <= id_col + half_size; ++x )
        {
          if ( map.depth( y, x ) < 0.0 ) // Check if neighbors point is valid before using it.
            continue;

          const openMVG::Vec3 p = pts( y, x ) - c;

          cov( 0, 0 ) += p.x() * p.x();
          cov( 0, 1 ) += p.x() * p.y();
          cov( 0, 2 ) += p.x() * p.z();

          // cov( 1, 0 ) = cov( 0, 1)
          cov( 1, 1 ) += p.y() * p.y();
          cov( 1, 2 ) += p.y() * p.z();

          // cov( 2 , 0 ) = cov( 0 , 2 )
          // cov( 2 , 1 ) = cov( 1 , 2 )
          cov( 2, 2 ) += p.z() * p.z();
        }
      }
      cov( 1, 0 ) = cov( 0, 1 );
      cov( 2, 0 ) = cov( 0, 2 );
      cov( 2, 1 ) = cov( 1, 2 );

      cov *= ( 1.0 / static_cast<double>( nb_ok ) );

      // 2.3 Solve for PCA
      Eigen::SelfAdjointEigenSolver<openMVG::Mat3> eigenSolver( cov );
      if ( eigenSolver.info() != Eigen::Success )
        continue;
      // Assuming eigen vectors are sorted in increasing eigenvalues, the smallest one is the normal
      // Note: eigenvectors are already normalized.
      openMVG::Vec3 n = eigenSolver.eigenvectors().col( 0 ).normalized();

      // 2.4 Orientation consistency ( it must look towards the camera )
      const openMVG::Vec3 dir = pts( id_row, id_col ) - cam.m_C;
      if ( dir.dot( n ) > 0.0 )
      {
        n = -n;
      }

      // 2.5 Store normal (recompute plane)
      const double        depth = map.depth( id_row, id_col );
      const double        d     = GetPlaneD( cam, id_row, id_col, depth, n, scale );
      const openMVG::Vec4 plane( n[ 0 ], n[ 1 ], n[ 2 ], d );

      map.plane( id_row, id_col, plane );
    }
  }
}

} // namespace MVS