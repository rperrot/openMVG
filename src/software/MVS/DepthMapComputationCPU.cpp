#include "DepthMapComputationCPU.hpp"

#include "MatchingCost.hpp"
#include "Util.hpp"

#include <random>
#include <array>

namespace MVS
{
  /**
  * @brief Compute cost value at specified pixel position
  * @param id_row Y coordinate of the pixel
  * @param id_col X coordinate of the pixel
  * @param H Homography that maps points in first image to points in the second image
  * @param params Computation parameters
  * @param image_ref Image data of the first view
  * @param image_other Image data of the second view
  * @return Pixel cost using NCC value
  */
  double ComputePixelCostNCC( const int id_row , const int id_col ,
                              const openMVG::Mat3 & H ,
                              const DepthMapComputationParameters & params ,
                              const Image & image_ref ,
                              const Image & image_other )
  {
    static constexpr int window = 11 ;
    static constexpr int half_w = window / 2 ;

    std::array< double , window * window > val_p ;
    std::array< double , window * window > val_q ;

    int id = 0 ;
    for( int y = id_row - half_w ; y <= id_row + half_w ; ++y )
    {
      for( int x = id_col - half_w ; x <= id_col + half_w ; ++x )
      {
        if( ! image_ref.Inside( y , x ) )
        {
          return DepthMapComputationParameters::MAX_COST_NCC ;
        }
        const openMVG::Vec3 p( x , y , 1.0 ) ;
        const openMVG::Vec3 q = H * p ;

        // Handle division by 0
        if( fabs( q[2] ) <= std::numeric_limits<double>::epsilon() )
        {
          return DepthMapComputationParameters::MAX_COST_NCC ;
        }

        const double inv = 1.0 / q[2] ;
        const int qx = q[0] * inv ;
        const int qy = q[1] * inv ;

        if( ! image_other.Inside( qy , qx ) )
        {
          return DepthMapComputationParameters::MAX_COST_NCC ;
        }

        val_p[ id ] = image_ref.Intensity( y , x ) ;
        val_q[ id ] = image_other.Intensity( qy , qx ) ;

        ++id ;
      }
    }

    const double mean_p = std::accumulate( val_p.begin() , val_p.begin() + id , 0.0 ) / static_cast<double>( val_p.size() ) ;
    const double mean_q = std::accumulate( val_q.begin() , val_q.begin() + id , 0.0 ) / static_cast<double>( val_q.size() ) ;

    double num = 0.0 ;
    double denom_p = 0.0 ;
    double denom_q = 0.0 ;
    for( size_t i = 0 ; i < id  ; ++i )
    {
      const double diff_p = val_p[i] - mean_p ;
      const double diff_q = val_q[i] - mean_q ;
      num += diff_p * diff_q ;
      denom_p += diff_p * diff_p ;
      denom_q += diff_q * diff_q ;
    }
    const double ncc = Clamp( num / std::sqrt( denom_p * denom_q ) , -1.0 , 1.0 ) ;

    return 1.0 - ncc ;
  }

  /**
  * @brief Compute pixel cost using Patch Match metric
  * @param id_row Y-coordinate of the pixel
  * @param id_col X-coordinate of the pixel
  * @param H Homography that maps points in first image to points in the second image
  * @param params Computation parameters
  * @param image_ref Image data of the first view
  * @param image_other Image data of the second view
  * @return Pixel cost using PatchMatch metric
  */
  double ComputePixelCostPM( const int id_row ,
                             const int id_col ,
                             const openMVG::Mat3 & H ,
                             const DepthMapComputationParameters & params ,
                             const Image & image_ref ,
                             const Image & image_other )
  {
    // TODO : bring it to a parameter
    const int window = 11 ;
    const int half_w = window / 2 ;

    const int Ic = image_ref.Intensity( id_row , id_col );
    static const std::vector<double> exp_table = GetExpTable( params.Gamma() ) ;
    std::vector< std::pair< openMVG::Vec2i , openMVG::Vec2i > > points_pair( window * window ) ;

    // Compute sampling positions
    int id = 0 ;
    for( int y = id_row - half_w ; y <= id_row + half_w ; ++y )
    {
      for( int x = id_col - half_w ; x <= id_col + half_w ; ++x )
      {
        if( ! image_ref.Inside( y , x ) )
        {
          return DepthMapComputationParameters::MAX_COST_PM ;
        }

        openMVG::Vec2i posP( x , y ) ;

        const openMVG::Vec3 p( x , y , 1.0 ) ;
        const openMVG::Vec3 q = H * p ;

        // Handle division by 0
        if( fabs( q[2] ) <= std::numeric_limits<double>::epsilon() )
        {
          return DepthMapComputationParameters::MAX_COST_PM ;
        }

        const double inv = 1.0 / q[2] ;

        const int qx = q[0] * inv ;
        const int qy = q[1] * inv ;

        if( ! image_other.Inside( qy , qx ) )
        {
          return DepthMapComputationParameters::MAX_COST_PM ;
        }

        openMVG::Vec2i posQ( qx , qy ) ;

        points_pair[ id ] = std::make_pair( posP , posQ ) ;
        ++id ;
      }
    }

    assert( id == window * window ) ;

    // Compute error
    double res = 0.0 ;
    for( size_t id_pair = 0 ; id_pair < id ; ++id_pair )
    {
      const openMVG::Vec2i & p = points_pair[ id_pair ].first ;
      const openMVG::Vec2i & q = points_pair[ id_pair ].second ;

      const unsigned char Ip = image_ref.Intensity( p[1] , p[0] ) ;
      const openMVG::Vec4 & Gp = image_ref.Gradient( p[1] , p[0] ) ;

      const unsigned char Iq = image_other.Intensity( q[1] , q[0] ) ;
      const openMVG::Vec4 & Gq = image_other.Gradient( q[1] , q[0] ) ;

      const int normI = std::abs( Ic - static_cast<int>( Ip ) ) ;
      const double w = exp_table[ normI ] ; // std::exp( - normI / params.Gamma() ) ;

      const double cost = ComputeMatchingCost( Ip , Gp , Iq , Gq , params.Alpha() , params.TauI() , params.TauG() ) ;

      res += w * cost ;
    }
    return res ;
  }


  /**
  * @brief compute cost between two images
  * @param[out] cost Cost for all pixels
  * @param planes The planes per pixel
  * @param reference_cam First view camera
  * @param other_cam Second view camera
  * @param stereo_rig Stereo parameters (Rotation, translation) motion from first to second view
  * @param image_ref Image data of the first view
  * @param image_other Image data of the second view
  * @param params Computation parameters
  * @param scale Optionnal scale of the computation (if not specified , used the user specified resolution)
  */
  void ComputeImagePairCost( openMVG::image::Image<double> & cost ,
                             const openMVG::image::Image<openMVG::Vec4> & planes ,
                             const Camera & reference_cam ,
                             const Camera & other_cam ,
                             const std::pair< openMVG::Mat3 , openMVG::Vec3 > & stereo_rig ,
                             const Image & image_ref ,
                             const Image & image_other ,
                             const DepthMapComputationParameters & params ,
                             const int scale )
  {
    double MAX_COST ;
    if( params.Metric() == COST_METRIC_NCC )
    {
      MAX_COST = DepthMapComputationParameters::MAX_COST_NCC ;
    }
    else
    {
      MAX_COST = DepthMapComputationParameters::MAX_COST_PM ;
    }

    cost.resize( image_ref.Width() , image_ref.Height() , true , MAX_COST ) ;

    const cost_metric cur_metric = params.Metric() ;

    for( int id_row = 0 ; id_row < image_ref.Height() ; ++id_row )
    {
      for( int id_col = 0 ; id_col < image_ref.Width() ; ++id_col )
      {
        // 1 - Compute homography at this pixel
        const openMVG::Vec4 & cur_plane = planes( id_row , id_col ) ;
        const openMVG::Mat3 H = HomographyTransformation( stereo_rig.first ,    // R
                                stereo_rig.second ,   // t
                                reference_cam ,   // first camera
                                other_cam ,       // second camera
                                cur_plane ,
                                scale ) ;

        // Compute cost at this pixel
        if( cur_metric == COST_METRIC_PM )
        {
          cost( id_row , id_col ) = ComputePixelCostPM( id_row , id_col , H , params , image_ref , image_other ) ;
        }
        else
        {
          cost( id_row , id_col ) = ComputePixelCostNCC( id_row , id_col , H , params , image_ref , image_other ) ;
        }
      }
    }
  }

  /**
  * @brief Compute cost using all neighboring images
  * @param[out] Final cost value for all pixels
  * @param planes The planes parameters for each pixel
  * @param reference_cam The reference view camera
  * @param cams Array of all cameras
  * @param stereo_rig Array of all stereo rigs between reference and it's neighboring
  * @param image_ref Image data of the reference view
  * @param neigh_imgs Neighboring images
  * @param params Computation parameters
  */
  void ComputeMultipleViewCost( openMVG::image::Image<double> & cost ,
                                const openMVG::image::Image<openMVG::Vec4> & planes ,
                                const Camera & reference_cam ,
                                const std::vector< Camera > & cams ,
                                const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                                const Image & image_ref ,
                                const std::vector< Image > & neigh_imgs ,
                                const DepthMapComputationParameters & params ,
                                const int scale )
  {
    std::vector< openMVG::image::Image<double> > all_costs( reference_cam.m_view_neighbors.size() ) ;

    // 1 compute cost for all images
    #pragma omp parallel for
    for( size_t id_cam = 0 ; id_cam < reference_cam.m_view_neighbors.size() ; ++id_cam )
    {
      const int id_neigh = reference_cam.m_view_neighbors[ id_cam ] ;
      const Camera & other_cam = cams[ id_neigh ] ;
      const std::pair< openMVG::Mat3 , openMVG::Vec3 > & cur_rig = stereo_rig[ id_cam ] ;

      ComputeImagePairCost( all_costs[ id_cam ] , planes , reference_cam , other_cam , cur_rig , image_ref , neigh_imgs[ id_cam ] , params , scale ) ;
    }

    double MAX_COST ;
    if( params.Metric() == COST_METRIC_NCC )
    {
      MAX_COST = DepthMapComputationParameters::MAX_COST_NCC ;
    }
    else
    {
      MAX_COST = DepthMapComputationParameters::MAX_COST_PM ;
    }
    cost.resize( image_ref.Width() , image_ref.Height() , true , MAX_COST ) ;

    // 2 Sort costs and compute final value
    const size_t K = params.NbMultiViewImageForCost() ;
    #pragma omp parallel for
    for( int id_row = 0 ; id_row < image_ref.Height() ; ++id_row )
    {
      std::vector< double > cur_costs( reference_cam.m_view_neighbors.size() ) ;

      for( int id_col = 0 ; id_col < image_ref.Width() ; ++id_col )
      {
        size_t nb_valid = 0 ;
        // 1 - retreive all costs
        for( size_t id_cam = 0 ; id_cam < reference_cam.m_view_neighbors.size() ; ++id_cam )
        {
          const double cur_c = all_costs[ id_cam ]( id_row , id_col ) ;
          const bool valid = cur_c < MAX_COST ;
          cur_costs[ id_cam ] =  valid ? cur_c : MAX_COST ;
          nb_valid += valid ? 1 : 0 ;
        }

        // 2 - Sort cost
        std::sort( cur_costs.begin() , cur_costs.end() ) ;

        double cur_sum = 0.0 ;
        //
        for( size_t id = 0 ; id < K && id < nb_valid ; ++id )
        {
          cur_sum += cur_costs[ id ] ;
        }

        // Compute final value
        cur_sum /= static_cast<double>( nb_valid ) ;
        if( nb_valid > 0 && cur_sum < MAX_COST && cur_sum >= 0.0 )
        {
          cost( id_row , id_col ) = cur_sum ;
        }
      }
    }
  }

  /**
  * @brief Compute multiple view cost for specified pixel
  * @param id_row Y-coordinate of the pixel
  * @param id_col X-coordinate of the pixel
  * @param cur_normal Normal of the plane
  * @param cur_d Plane parameter
  * @param cams Array of all neighboring cameras
  * @param stereo_rig Stereo motion wrt all neighboring cameras
  * @param image_ref Image data of the reference view
  * @param neigh_imgs Image data of all the neighbors of the camera
  * @param params Computation parameters
  * @return Multiple view cost for the specified pixel
  */
  double ComputeMultiViewCost( const int id_row , const int id_col ,
                               const openMVG::Vec3 & cur_normal , // Normal parameter
                               const double & cur_d , // Plane d
                               const Camera & reference_cam ,
                               const std::vector< Camera > & cams ,
                               const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                               const Image & image_ref ,
                               const std::vector< Image > & neigh_imgs ,
                               const DepthMapComputationParameters & params ,
                               const int scale )
  {
    double MAX_COST ;
    if( params.Metric() == COST_METRIC_NCC )
    {
      MAX_COST = DepthMapComputationParameters::MAX_COST_NCC ;
    }
    else
    {
      MAX_COST = DepthMapComputationParameters::MAX_COST_PM ;
    }


    const int K = params.NbMultiViewImageForCost() ;
    int nb_valid = 0 ;

    std::vector< double > costs( reference_cam.m_view_neighbors.size() ) ;

    // Get reference intrinsic
    const openMVG::Mat3 & reference_K = ( scale == -1 ) ? reference_cam.m_K : reference_cam.m_K_scaled[ scale ] ;

    // Compute cost for all camera
    for( size_t id_cam = 0 ; id_cam < reference_cam.m_view_neighbors.size() ; ++id_cam )
    {
      const int id_neigh = reference_cam.m_view_neighbors[ id_cam ] ;
      const Camera & cur_cam = cams[ id_neigh ] ;
      const std::pair< openMVG::Mat3 , openMVG::Vec3 > & cur_rig = stereo_rig[ id_cam ] ;

      // Get neigh intrinsic matrix
      const openMVG::Mat3 & cur_K = ( scale == -1 ) ? cur_cam.m_K : cur_cam.m_K_scaled[ scale ] ;

      const openMVG::Mat3 H = HomographyTransformation( cur_rig.first ,     // R
                              cur_rig.second ,    // t
                              reference_cam.m_K , // first camera
                              cur_K ,       // second camera
                              cur_normal ,        // plane normal
                              cur_d ) ;           // plane parameter

      double cur_cost ;
      // Compute cost at specified position
      if( params.Metric() == COST_METRIC_NCC )
      {
        cur_cost = ComputePixelCostNCC( id_row , id_col , H , params , image_ref , neigh_imgs[ id_cam ] ) ;
      }
      else
      {
        cur_cost = ComputePixelCostPM( id_row , id_col , H , params , image_ref , neigh_imgs[ id_cam ] ) ;
      }

      if( cur_cost < MAX_COST )
      {
        costs[ id_cam ] = cur_cost ;
        nb_valid ++ ;
      }
      else
      {
        costs[ id_cam ] = MAX_COST ;
      }
    }

    // Sort cost and compute sum of the K first
    std::sort( costs.begin() , costs.end() ) ;

    double cost = 0.0 ;
    int nb = 0 ;
    for( int k = 0 ; k < K && k < static_cast<int>( costs.size() ) && k < nb_valid ; ++k )
    {
      cost += costs[ k ] ;
      ++nb ;
    }

    cost /= static_cast<double>( nb ) ;

    // No valid
    if( nb == 0 || std::isnan( cost ) || std::isinf( cost ) || cost < 0.0 )
    {
      return MAX_COST ;
    }
    return cost ;
  }

  /**
  * @brief Compute initial matching cost of specified camera
  * @param map The depth map of the reference image
  * @param reference_cam The reference view camera
  * @param cams Array of neighboring cameras
  * @param stereo_rig Array of all motions between reference and it's neighbors
  * @param image_ref Reference image
  * @param params Computation parameters
  */
  void ComputeCost( DepthMap & map ,
                    const Camera & reference_cam ,
                    const std::vector< Camera > & cams ,
                    const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                    const Image & image_ref ,
                    const DepthMapComputationParameters & params )
  {
    // Load images for the neighbors
    std::vector< Image > neigh_imgs = LoadNeighborImages( reference_cam , params ) ;

    openMVG::image::Image<double> costs ;
    const openMVG::image::Image<openMVG::Vec4> & planes = map.Planes() ;

    // 1 - Compute cost
    ComputeMultipleViewCost( costs , planes , reference_cam , cams , stereo_rig , image_ref , neigh_imgs , params ) ;

    // 2 - Store back cost
    for( int id_row = 0 ; id_row < image_ref.Height() ; ++id_row )
    {
      for( int id_col = 0 ; id_col < image_ref.Width() ; ++id_col )
      {
        map.Cost( id_row , id_col , costs( id_row , id_col ) ) ;
      }
    }
  }

  /**
  * @brief Compute initial cost at a specific scale
  * @param map The depth map of the reference image
  * @param reference_cam The reference view camera
  * @param cams Array of neighboring cameras
  * @param stereo_rig Array of all motions between reference and it's neighbors
  * @param image_ref Reference image
  * @param neigh_imgs Neighboring images
  * @param params Computation parameters
  * @param scale Scale of the computation
  */
  void ComputeCost( DepthMap & map ,
                    const Camera & reference_cam ,
                    const std::vector< Camera > & cams ,
                    const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                    const Image & image_ref ,
                    const std::vector< Image > & neigh_imgs ,
                    const DepthMapComputationParameters & params ,
                    const int scale )
  {
    openMVG::image::Image<double> costs ;
    const openMVG::image::Image<openMVG::Vec4> & planes = map.Planes() ;

    // 1 - Compute cost
    ComputeMultipleViewCost( costs , planes , reference_cam , cams , stereo_rig , image_ref , neigh_imgs , params , scale ) ;

    // 2 - Store back cost
    for( int id_row = 0 ; id_row < image_ref.Height() ; ++id_row )
    {
      for( int id_col = 0 ; id_col < image_ref.Width() ; ++id_col )
      {
        map.Cost( id_row , id_col , costs( id_row , id_col ) ) ;
      }
    }
  }


  /**
  * @brief Perform propagation using Red or Black scheme
  * @param[in,out] map The depth map to optimize
  * @param id_start 0 if propagate Red , 1 if propagate Black
  * @param cam Reference camera
  * @param cams Neighboring cameras
  * @param stereo_rig Array of motion between reference and it's neighbors
  * @param image_ref Image data of the reference view
  * @param params Computation parameters
  */
  void Propagate( DepthMap & map , const int id_start ,
                  Camera & cam ,
                  const std::vector< Camera > & cams ,
                  const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                  const Image & image_ref ,
                  const DepthMapComputationParameters & params )
  {
    // Load images for the neighbors
    std::vector< Image > neigh_imgs = LoadNeighborImages( cam , params ) ;


    // (x,y)
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
    const int neighs_idx[20][2] =
    {
      {0, -5} ,

      {0, -3} ,

      { -1, -2} ,
      {1, -2} ,

      { -2, -1} ,
      {0, -1} ,
      {2, -1} ,

      { -5, 0} ,
      { -3, 0} ,
      { -1, 0} ,
      {1, 0} ,
      {3, 0} ,
      {5, 0} ,

      { -2, 1} ,
      {0, 1} ,
      {2, 1} ,

      { -1, 2} ,
      {1, 2} ,

      {0, 3} ,

      {0, 5}
    } ;

    #pragma omp parallel for
    for( int id_row = 0 ; id_row < map.Height() ; ++id_row )
    {
      const int pad = ( id_row % 2 == 0 ) ? id_start : ( id_start + 1 ) % 2 ;
      for( int id_col = pad ; id_col < map.Width() ; id_col += 2  )
      {
        // Get neighbors using
        for( int id_n = 0 ; id_n < 20 ; ++id_n )
        {
          const int x = id_col + neighs_idx[ id_n ][ 0 ] ;
          const int y = id_row + neighs_idx[ id_n ][ 1 ] ;

          if( map.Inside( y , x ) )
          {
            // Compute cost at given pixel using the other planes
            const openMVG::Vec4 & plane      = map.Plane( y , x ) ;
            const openMVG::Vec3 plane_n( plane[0] , plane[1] , plane[2] ) ;
            const double plane_d           = plane[3] ;

            // Given the depth, compute the d value of the plane (ie intersection between ray and the plane)
            const double new_cost = ComputeMultiViewCost( id_row , id_col , plane_n , plane_d , cam , cams , stereo_rig , image_ref , neigh_imgs , params ) ;

            if( new_cost < map.Cost( id_row , id_col ) )
            {
              // Copy cost
              map.Cost( id_row , id_col , new_cost ) ;
              // Copy plane
              map.Plane( id_row , id_col , plane ) ;

              // Compute current depth at this pixel
              const double z = ComputeDepth( plane , id_row , id_col , cam ) ;
              map.Depth( id_row , id_col , z ) ;
            }
          }
        }
      }
    }
  }


  /**
  * @brief Perform propagation using Red or Black scheme at specific scale
  * @param[in,out] map The depth map to optimize
  * @param id_start 0 if propagate Red , 1 if propagate Black
  * @param cam Reference camera
  * @param cams Neighboring cameras
  * @param stereo_rig Array of motion between reference and it's neighbors
  * @param image_ref Image data of the reference view
  * @param params neigh_imgs Neighboring images
  * @param params Computation parameters
  * @param scale Scale of the computation
  */
  void Propagate( DepthMap & map , const int id_start ,
                  const Camera & cam ,
                  const std::vector< Camera > & cams ,
                  const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                  const Image & image_ref ,
                  const std::vector< Image > & neigh_imgs ,
                  const DepthMapComputationParameters & params ,
                  const int scale )
  {
    // (x,y)
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
    const int neighs_idx[20][2] =
    {
      {0, -5} ,

      {0, -3} ,

      { -1, -2} ,
      {1, -2} ,

      { -2, -1} ,
      {0, -1} ,
      {2, -1} ,

      { -5, 0} ,
      { -3, 0} ,
      { -1, 0} ,
      {1, 0} ,
      {3, 0} ,
      {5, 0} ,

      { -2, 1} ,
      {0, 1} ,
      {2, 1} ,

      { -1, 2} ,
      {1, 2} ,

      {0, 3} ,

      {0, 5}
    } ;

    #pragma omp parallel for
    for( int id_row = 0 ; id_row < map.Height() ; ++id_row )
    {
      const int pad = ( id_row % 2 == 0 ) ? id_start : ( id_start + 1 ) % 2 ;
      for( int id_col = pad ; id_col < map.Width() ; id_col += 2  )
      {
        // Get neighbors using
        for( int id_n = 0 ; id_n < 20 ; ++id_n )
        {
          const int x = id_col + neighs_idx[ id_n ][ 0 ] ;
          const int y = id_row + neighs_idx[ id_n ][ 1 ] ;

          if( map.Inside( y , x ) )
          {
            // Compute cost at given pixel using the other planes
            const openMVG::Vec4 & plane      = map.Plane( y , x ) ;
            const openMVG::Vec3 plane_n( plane[0] , plane[1] , plane[2] ) ;
            const double plane_d           = plane[3] ;

            // Given the depth, compute the d value of the plane (ie intersection between ray and the plane)
            const double new_cost = ComputeMultiViewCost( id_row , id_col , plane_n , plane_d , cam , cams , stereo_rig , image_ref , neigh_imgs , params , scale ) ;

            if( new_cost < map.Cost( id_row , id_col ) )
            {
              // Copy cost
              map.Cost( id_row , id_col , new_cost ) ;
              // Copy plane
              map.Plane( id_row , id_col , plane ) ;

              // Compute current depth at this pixel
              const double z = ComputeDepth( plane , id_row , id_col , cam ) ;
              map.Depth( id_row , id_col , z ) ;
            }
          }
        }
      }
    }
  }


  /**
  * @brief Perform plane refinement
  * @param map Depth map to refine
  * @param cam Reference camera
  * @param cams Array of all neighboring cameras
  * @param stereo_rig Array of motion between reference and its neighbors
  * @param image_ref Image data of the reference view
  * @param params Computation parameters
  */
  void Refinement( DepthMap & map ,
                   const Camera & cam ,
                   const std::vector< Camera > & cams ,
                   const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                   const Image & image_ref ,
                   const DepthMapComputationParameters & params )
  {
    // Load images for the neighbors
    const std::vector< Image > neigh_imgs = LoadNeighborImages( cam , params ) ;

    // Initialize RNG
    std::mt19937_64 rng ;
    std::random_device device;
    std::seed_seq seq{device(), device(), device(), device()};
    rng.seed( seq ) ;

    std::uniform_real_distribution<double> distrib_01( 0.0 , 1.0 ) ;

    // TODO: use another value (more data oriented)
    const double disparity_threshold = 0.01 ;


    #pragma omp parallel for
    for( int id_row = 0 ; id_row < map.Height() ; ++id_row )
    {
      for( int id_col = 0 ; id_col < map.Width() ; ++id_col )
      {
        openMVG::Vec3 cam_dir = cam.GetViewVector( id_col , id_row ) ;  //  cam.GetRay( openMVG::Vec2( id_col , id_row ) ).second ;

        const double min_disparity = cam.DepthDisparityConversion( cam.m_max_depth ) ;
        const double max_disparity = cam.DepthDisparityConversion( cam.m_min_depth ) ;

        double delta_disparity = max_disparity / 2.0 ;
        double delta_N = 1.0 ;

        while( delta_disparity > disparity_threshold )
        {
          const double u1 = distrib_01( rng ) ;
          const double u2 = distrib_01( rng ) ;
          const double u3 = distrib_01( rng ) ;
          const double u4 = distrib_01( rng ) ;


          // Compute new depth :
          const openMVG::Vec4 & plane = map.Plane( id_row , id_col ) ;
          const openMVG::Vec3 cur_n( plane[0] , plane[1] , plane[2] ) ;
          const double        cur_d = map.Depth( id_row , id_col ) ;
          const double        cur_disp = cam.DepthDisparityConversion( cur_d ) ;

          const double min_delta  = - std::min( delta_disparity , cur_disp + min_disparity ) ;
          const double max_delta  = std::min( delta_disparity , max_disparity - cur_disp ) ;
          const double delta_disp = u1 * ( max_delta - min_delta ) + min_delta ;
          const double new_disp   = Clamp( cur_disp + delta_disp , min_disparity , max_disparity ) ;
          const double new_d      = cam.DepthDisparityConversion( new_disp ) ;

          // Compute new normal
          const double r1 = ( u2 * delta_N * 2.0 ) - delta_N ;
          const double r2 = ( u3 * delta_N * 2.0 ) - delta_N ;
          const double r3 = ( u4 * delta_N * 2.0 ) - delta_N ;

          openMVG::Vec3 new_n = cur_n + openMVG::Vec3( r1 , r2 , r3 ) ;
          new_n = new_n.normalized() ;

          // Handle vector in same range of the view vector
          if( cam_dir.dot( new_n ) > 0.0 )
          {
            new_n = - new_n ;
          }

          // Compute plane d
          const double d_plane    = GetPlaneD( cam , id_row , id_col , new_d , new_n ) ;

          // Compute cost
          const double new_cost = ComputeMultiViewCost( id_row , id_col , new_n , d_plane , cam , cams , stereo_rig , image_ref , neigh_imgs , params ) ;

          if( new_cost < map.Cost( id_row , id_col ) )
          {
            // Update value
            map.Cost( id_row , id_col , new_cost ) ;
            map.Plane( id_row , id_col , openMVG::Vec4( new_n[0] , new_n[1] , new_n[2] , d_plane ) ) ;
            map.Depth( id_row , id_col , new_d ) ;
          }

          // Halve the range
          delta_disparity /= 2.0 ;
          delta_N /= 2.0 ;
        }
      }
    }
  }

  /**
  * @brief Perform plane refinement at specific scale
  * @param map Depth map to refine
  * @param cam Reference camera
  * @param cams Array of all neighboring cameras
  * @param stereo_rig Array of motion between reference and its neighbors
  * @param image_ref Image data of the reference view
  * @param params Computation parameters
  */
  void Refinement( DepthMap & map ,
                   const Camera & cam ,
                   const std::vector< Camera > & cams ,
                   const std::vector< std::pair< openMVG::Mat3 , openMVG::Vec3 > > & stereo_rig ,
                   const Image & image_ref ,
                   const std::vector< Image > & neigh_imgs ,
                   const DepthMapComputationParameters & params ,
                   const int scale )
  {
    // Initialize RNG
    std::mt19937_64 rng ;
    std::random_device device;
    std::seed_seq seq{device(), device(), device(), device()};
    rng.seed( seq ) ;

    std::uniform_real_distribution<double> distrib_01( 0.0 , 1.0 ) ;

    // TODO: use another value (more data oriented)
    const double disparity_threshold = 0.01 ;

    #pragma omp parallel for
    for( int id_row = 0 ; id_row < map.Height() ; ++id_row )
    {
      for( int id_col = 0 ; id_col < map.Width() ; ++id_col )
      {
        openMVG::Vec3 cam_dir = cam.GetViewVector( id_col , id_row ) ;  //  cam.GetRay( openMVG::Vec2( id_col , id_row ) ).second ;

        const double min_disparity = cam.DepthDisparityConversion( cam.m_max_depth ) ;
        const double max_disparity = cam.DepthDisparityConversion( cam.m_min_depth ) ;

        double delta_disparity = max_disparity / 2.0 ;
        double delta_N = 1.0 ;

        while( delta_disparity > disparity_threshold )
        {
          const double u1 = distrib_01( rng ) ;
          const double u2 = distrib_01( rng ) ;
          const double u3 = distrib_01( rng ) ;
          const double u4 = distrib_01( rng ) ;


          // Compute new depth :
          const openMVG::Vec4 & plane = map.Plane( id_row , id_col ) ;
          const openMVG::Vec3 cur_n( plane[0] , plane[1] , plane[2] ) ;
          const double        cur_d = map.Depth( id_row , id_col ) ;
          const double        cur_disp = cam.DepthDisparityConversion( cur_d ) ;

          const double min_delta  = - std::min( delta_disparity , cur_disp + min_disparity ) ;
          const double max_delta  = std::min( delta_disparity , max_disparity - cur_disp ) ;
          const double delta_disp = u1 * ( max_delta - min_delta ) + min_delta ;
          const double new_disp   = Clamp( cur_disp + delta_disp , min_disparity , max_disparity ) ;
          const double new_d      = cam.DepthDisparityConversion( new_disp ) ;

          // Compute new normal
          const double r1 = ( u2 * delta_N * 2.0 ) - delta_N ;
          const double r2 = ( u3 * delta_N * 2.0 ) - delta_N ;
          const double r3 = ( u4 * delta_N * 2.0 ) - delta_N ;

          openMVG::Vec3 new_n = cur_n + openMVG::Vec3( r1 , r2 , r3 ) ;
          new_n = new_n.normalized() ;

          // Handle vector in same range of the view vector
          if( cam_dir.dot( new_n ) > 0.0 )
          {
            new_n = - new_n ;
          }

          // Compute plane d
          const double d_plane    = GetPlaneD( cam , id_row , id_col , new_d , new_n ) ;

          // Compute cost
          const double new_cost = ComputeMultiViewCost( id_row , id_col , new_n , d_plane , cam , cams , stereo_rig , image_ref , neigh_imgs , params , scale ) ;

          if( new_cost < map.Cost( id_row , id_col ) )
          {
            // Update value
            map.Cost( id_row , id_col , new_cost ) ;
            map.Plane( id_row , id_col , openMVG::Vec4( new_n[0] , new_n[1] , new_n[2] , d_plane ) ) ;
            map.Depth( id_row , id_col , new_d ) ;
          }

          // Halve the range
          delta_disparity /= 2.0 ;
          delta_N /= 2.0 ;
        }
      }
    }
  }

}