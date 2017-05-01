#ifndef OPENMVG_GEOMETRY_REGISTRATION_ICP_HPP
#define OPENMVG_GEOMETRY_REGISTRATION_ICP_HPP

#include "openMVG/geometry/kd_tree_3d.hpp"
#include "openMVG/geometry/registration/rigid_motion_3d_3d_estimation.hpp"
#include "openMVG/numeric/numeric.h"

#include <flann/flann.hpp>

#include <memory>
#include <numeric>
#include <random>

namespace openMVG
{
namespace geometry
{
namespace registration
{


template<typename Scalar>
void Transform( Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> & data , const Eigen::Quaternion<Scalar> & q , Eigen::Vector3d & t )
{
  for( int id_pt = 0 ; id_pt < data.rows() ; ++id_pt )
  {
    Eigen::Matrix<Scalar, 3, 1> cur_pt;
    cur_pt << data( id_pt , 0 ) , data( id_pt , 1 ) , data( id_pt , 2 ) ;
    const Eigen::Matrix<Scalar, 3, 1> tra_pt = ( q * cur_pt ) + t ;
    data( id_pt , 0 ) = tra_pt[0] ;
    data( id_pt , 1 ) = tra_pt[1] ;
    data( id_pt , 2 ) = tra_pt[2] ;
  }
}

/**
* @brief Compute mean square error between two set of points
* @param target target point list
* @param data source point list
* @param corresp pair (index, corresp[index]) for error computation
* @retval mse if number of pair is > 0
* @retval +inf if number of pair == 0
*/
template< typename Scalar>
Scalar ComputeMSE( const Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> &target,
                   const Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> &data ,
                   const std::vector<int> & corresp )
{
  int nb_valid = 0 ;
  Scalar mse = 0 ;
  for ( int id_pt = 0; id_pt < data.rows(); ++id_pt )
  {
    const int corr = corresp[ id_pt ];
    if ( corr >= 0 )
    {
      const Scalar d[] = {data( id_pt, 0 ) - target( corr, 0 ),
                          data( id_pt, 1 ) - target( corr, 1 ),
                          data( id_pt, 2 ) - target( corr, 2 )
                         };

      // Add distance between the two points
      mse += ( d[ 0 ] * d[ 0 ] +
               d[ 1 ] * d[ 1 ] +
               d[ 2 ] * d[ 2 ] );
      ++nb_valid;
    }
  }
  if( nb_valid > 0 )
  {
    return mse / static_cast<Scalar>( nb_valid );
  }
  else
  {
    return std::numeric_limits<Scalar>::max() ;
  }
}

/**
* @brief Compute a random subset of a list
* @param highest_value Maximum index for the output
* @param percentage Percentage of value to keep in the set [0;highest_value]
* @param[out] samples Output samples
* @param rng Random number generator
* @todo : move to numeric ?
*/
static inline void RandomSubset( const size_t highest_value , // included
                                 const double percentage , // between 0 and 1
                                 std::vector< int > & samples , // selected samples
                                 std::mt19937_64 & rng ) // random generator
{
  std::vector< int > res( highest_value ) ;
  std::iota( res.begin() , res.end() , 0 ) ;
  std::shuffle( res.begin() , res.end() , rng ) ;

  const int nb_values = std::min( static_cast<int>( highest_value ) , static_cast<int>( std::ceil( static_cast<double>( highest_value ) * percentage ) ) ) ;
  samples.resize( nb_values ) ;
  std::copy( res.begin() , res.begin() + nb_values , samples.begin() ) ;
}

/**
* @brief Compute standard deviation of a given set
* @param v a vector
* @return standard deviation of the vector
* @todo : move to numeric ?
* @note : this assume that at least one value is in the set
*/
template< typename Scalar >
static inline Scalar StdDev( const std::vector< Scalar > & v )
{
  double sum = std::accumulate( v.begin(), v.end(), Scalar( 0 ) );
  double mean = sum / v.size();

  std::vector<double> diff( v.size() );
  std::transform( v.begin(), v.end(), diff.begin(),
                  std::bind2nd( std::minus<double>(), mean ) );
  double sq_sum = std::inner_product( diff.begin(), diff.end(), diff.begin(), Scalar( 0 ) );
  return std::sqrt( sq_sum / v.size() );
}


/**
* @brief Given two sets of points: target and data
* This function computes rigid transformation that maps model on target minimizing MSE distance
* @param target Target shape : a matrix of 3d points (one point per row)
* @param data data shape : a matrix of 3d points (one point per row)
* @param nb_iteration Maximum number of iteration
* @param mse_threshold Threshold use to stop computation
* @param[out] t Translation transformation (3d vector)
* @param[out] R Rotation transformation (3x3 rotation matrix)
*/
template <typename Scalar>
void ICP( const Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> &target,
          const Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> &data_,
          const unsigned long max_nb_iteration,
          const Scalar mse_threshold,
          openMVG::Vec3 &t,
          openMVG::Mat3 &R )
{

  // Build Kd-Tree
  KDTree3d<Scalar> tree( target );

  unsigned long id_iteration = 0;
  Scalar cur_mse             = std::numeric_limits<Scalar>::max();

  // Working sample
  Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> data = data_;

  Mat4 final_tra;
  final_tra.setIdentity();

  std::vector<int> corresp( data.rows() );
  std::vector<Scalar> distance( data.rows() );

  Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> indices;
  Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> dists;

  std::vector< int > subset_indice ;
  const double percentage = 0.10 ; // only sample 10% of the subset
  std::mt19937_64 rng ;
  Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> subset_data ;

  while ( id_iteration < max_nb_iteration && cur_mse > mse_threshold )
  {
    std::fill( corresp.begin(), corresp.end(), -1 );
    std::fill( distance.begin(), distance.end(), Scalar( -1.0 ) );

    // 1 - Pick a random subset
    RandomSubset( data.rows() - 1 , percentage , subset_indice , rng ) ;
    const int nb_subset_elts = std::min( static_cast<int>( data.rows() ) - 1 , static_cast<int>( subset_indice.size() ) ) ;
    subset_data.resize( nb_subset_elts , 3 ) ;
    for( size_t id_sample = 0 ; id_sample < subset_indice.size() ; ++id_sample )
    {
      const int indice = subset_indice[ id_sample ] ;
      subset_data( id_sample , 0 ) = data( indice , 0 ) ;
      subset_data( id_sample , 1 ) = data( indice , 1 ) ;
      subset_data( id_sample , 2 ) = data( indice , 2 ) ;
    }

    // 2 - Establish pairs based on nearest neighbor search
    tree.Search( subset_data , 1 , indices, dists );
    std::vector< Scalar > compact_dist( subset_data.rows() ) ;
    int id_dist = 0 ;
    for ( int id_pt = 0; id_pt < subset_data.rows(); ++id_pt )
    {
      const int id_point = indices( id_pt, 0 );

      if ( ( id_point >= 0 ) &&
           ( id_point < target.rows() ) )
      {
        distance[ id_pt ] = dists( id_pt, 0 );
        corresp[ id_pt ]  = id_point;
        compact_dist[ id_dist ] = dists( id_pt, 0 );

        ++id_dist ;
      }
    }

    // 3 - Filter points based on 3 * stddev
    const Scalar t = 3.0 * StdDev( compact_dist ) ;
    for ( int id_pt = 0; id_pt < subset_data.rows(); ++id_pt )
    {
      if ( corresp[ id_pt ] >= 0 )
      {
        if ( distance[ id_pt ] > t )
        {
          corresp[ id_pt ]  = -1;
          distance[ id_pt ] = -1.0;
        }
      }
    }
    const double mse_before = ComputeMSE( target , subset_data , corresp ) ;
    if( id_iteration == 0 )
    {
      cur_mse = mse_before ;
    }

    // 4 - Compute best rigid transformation based on pairs
    RigidMotion3d3dEstimation<Scalar> estimator ;
    std::pair< Eigen::Quaternion<Scalar> , Eigen::Matrix<Scalar, 3, 1> > tra = estimator( target , subset_data , corresp ) ;

    // 5 - Update data points and final transformation
    Transform( subset_data , tra.first , tra.second );
    const double mse_after = ComputeMSE( target , subset_data , corresp ) ;
    if( mse_after < mse_before )
    {
      // Update the whole set
      Transform( data , tra.first , tra.second );

      // Update global transformation
      Mat4 tmp = Mat4::Identity() ;
      tmp.block( 0 , 0 , 3 , 3 ) = tra.first.toRotationMatrix() ;
      tmp( 0 , 3 ) = tra.second[0] ;
      tmp( 1 , 3 ) = tra.second[1] ;
      tmp( 2 , 3 ) = tra.second[2] ;
      final_tra = tmp * final_tra ;
      // Update mse
      cur_mse = mse_after ;
    }

    ++id_iteration;
  }

  // Compute final transformation
  R = final_tra.block( 0 , 0 , 3 , 3 ) ;

  t[ 0 ] = final_tra( 0, 3 );
  t[ 1 ] = final_tra( 1, 3 );
  t[ 2 ] = final_tra( 2, 3 );
}


/**
* @brief Given two sets of points: target and data
* This function computes rigid transformation that maps model on target minimizing MSE distance
* This use the point to normal distance for computation
* @param target Target shape : a matrix of 3d points (one point per row)
* @param target_n Target shape normals : a matrix of 3d vectors (one vector per row)
* @param data data shape : a matrix of 3d points (one point per row)
* @param nb_iteration Maximum number of iteration
* @param mse_threshold Threshold use to stop computation
* @param[out] t Translation transformation (3d vector)
* @param[out] R Rotation transformation (3x3 rotation matrix)
*/
template <typename Scalar>
void ICP( const Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> &target,
          const Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> &target_n,
          const Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> &data_,
          const unsigned long max_nb_iteration,
          const Scalar mse_threshold,
          openMVG::Vec3 &t,
          openMVG::Mat3 &R )
{

  // Build Kd-Tree
  KDTree3d<Scalar> tree( target );

  unsigned long id_iteration = 0;
  Scalar cur_mse             = std::numeric_limits<Scalar>::max();

  // Working sample
  Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> data = data_;

  Mat4 final_tra;
  final_tra.setIdentity();

  //  std::vector<bool> already_used( target.rows() );
  std::vector<int> corresp( data.rows() );
  std::vector<Scalar> distance( data.rows() );

  Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> indices;
  Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> dists;


  std::vector< int > subset_indice ;
  const double percentage = 0.10 ; // only sample 10% of the subset
  std::mt19937_64 rng ;
  Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::RowMajor> subset_data ;

  while ( id_iteration < max_nb_iteration && cur_mse > mse_threshold )
  {
    std::fill( corresp.begin(), corresp.end(), -1 );
    std::fill( distance.begin(), distance.end(), Scalar( -1.0 ) );

    // 1 - Pick a random subset
    RandomSubset( data.rows() - 1 , percentage , subset_indice , rng ) ;
    const int nb_subset_elts = std::min( static_cast<int>( data.rows() ) - 1 , static_cast<int>( subset_indice.size() ) ) ;
    subset_data.resize( nb_subset_elts , 3 ) ;
    for( size_t id_sample = 0 ; id_sample < subset_indice.size() ; ++id_sample )
    {
      const int indice = subset_indice[ id_sample ] ;
      subset_data( id_sample , 0 ) = data( indice , 0 ) ;
      subset_data( id_sample , 1 ) = data( indice , 1 ) ;
      subset_data( id_sample , 2 ) = data( indice , 2 ) ;
    }

    // 2 - Establish pairs based on nearest neighbor search
    tree.Search( subset_data , 1 , indices, dists );
    std::vector< Scalar > compact_dist( subset_data.rows() ) ;
    int id_dist = 0 ;
    for ( int id_pt = 0; id_pt < subset_data.rows(); ++id_pt )
    {
      const int id_point = indices( id_pt, 0 );

      if ( ( id_point >= 0 ) &&
           ( id_point < target.rows() ) )
      {
        distance[ id_pt ] = dists( id_pt, 0 );
        corresp[ id_pt ]  = id_point;
        compact_dist[ id_dist ] = dists( id_pt, 0 );

        ++id_dist ;
      }
    }

    // 3 - Filter points based on 3 * stddev
    const Scalar t = 3.0 * StdDev( compact_dist ) ;
    for ( int id_pt = 0; id_pt < subset_data.rows(); ++id_pt )
    {
      if ( corresp[ id_pt ] >= 0 )
      {
        if ( distance[ id_pt ] > t )
        {
          corresp[ id_pt ]  = -1;
          distance[ id_pt ] = -1.0;
        }
      }
    }
    const double mse_before = ComputeMSE( target , subset_data , corresp ) ;
    if( id_iteration == 0 )
    {
      cur_mse = mse_before ;
    }

    // 4 - Compute best rigid transformation based on pairs
    RigidMotion3d3dEstimation<Scalar> estimator ;
    std::pair< Eigen::Quaternion<Scalar> , Eigen::Matrix<Scalar, 3, 1> > tra = estimator( target , target_n , subset_data , corresp ) ;

    // 5 - Update data points and final transformation
    Transform( subset_data , tra.first , tra.second );
    const double mse_after = ComputeMSE( target , subset_data , corresp ) ;
    if( mse_after < mse_before )
    {
      // Update whole set
      Transform( data , tra.first , tra.second );

      // Update global transformation
      Mat4 tmp = Mat4::Identity() ;
      tmp.block( 0 , 0 , 3 , 3 ) = tra.first.toRotationMatrix() ;
      tmp( 0 , 3 ) = tra.second[0] ;
      tmp( 1 , 3 ) = tra.second[1] ;
      tmp( 2 , 3 ) = tra.second[2] ;
      final_tra = tmp * final_tra ;

      // Update mse
      cur_mse = mse_after ;
    }

    ++id_iteration;
  }

  // Compute final transformation
  R = final_tra.block( 0 , 0 , 3 , 3 ) ;

  t[ 0 ] = final_tra( 0, 3 );
  t[ 1 ] = final_tra( 1, 3 );
  t[ 2 ] = final_tra( 2, 3 );
}



} // namespace registration
} // namespace geometry
} // namespace openMVG

#endif