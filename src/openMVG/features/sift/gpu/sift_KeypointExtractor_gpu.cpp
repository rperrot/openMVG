#include "sift_KeypointExtractor_gpu.hpp"

#include "openMVG/image/gpu/image_gpu_arithmetics.hpp"
#include "openMVG/image/gpu/image_gpu_interface.hpp"
#include "openMVG/image/gpu/image_gpu_local_maxima.hpp"

namespace openMVG
{
namespace features
{
namespace sift
{
namespace gpu
{

/**
* @brief SIFT_KeypointExtractor constructor
* @param peak_threshold Threshold on DoG operator
* @param edge_threshold Threshold on the ratio of principal curvatures
* @param nb_refinement_step Maximum number of refinement step to find exact location of interest point
*/
SIFT_KeypointExtractorGPU::SIFT_KeypointExtractorGPU
(
  float peak_threshold,     // i.e => 0.04 / slices per octave
  float edge_threshold,     // i.e => 10
  int nb_refinement_step ,
  const openMVG::system::gpu::OpenCLContext & ctx )
  : m_peak_threshold( peak_threshold ),
    m_edge_threshold( edge_threshold ),
    m_nb_refinement_step( nb_refinement_step ) ,
    m_ctx( ctx )
{
}


/**
* @brief Detect Scale Invariant points using Difference of Gaussians
* @param octave A Gaussian octave
* @param[out] keypoints The found Scale Invariant keypoint
*/
void SIFT_KeypointExtractorGPU::operator()( const openMVG::features::gpu::GPUOctave & octave , std::vector<Keypoint> & keypoints )
{
  if ( !ComputeDogs( octave ) )
  {
    return;
  }
  Find_and_refine_keypoints( keypoints , 0.8f ) ;
  /*
  Find_3d_discrete_extrema( keypoints, 0.8f );
  Keypoints_refine_position( keypoints );
  */
}

/**
* @brief Compute the Difference of Gaussians (Dogs) for a Gaussian octave
* @param Octave The input Gaussian octave
* @retval true If Dogs have been computed
* @retval false if Dogs cannot be computed (less than 2 images)
*/
bool SIFT_KeypointExtractorGPU::ComputeDogs( const openMVG::features::gpu::GPUOctave & octave )
{
  const int n = octave.slices.size();
  if ( n < 2 )
  {
    return false;
  }

  m_Dogs.slices.resize( n - 1 );

  m_Dogs.octave_level = octave.octave_level;
  m_Dogs.delta = octave.delta;
  m_Dogs.sigmas = octave.sigmas;


  if( octave.octave_level == 0 )
  {
    m_Dogs_gpu.slices.resize( n - 1 ) ;
    for( int id_slice = 0 ; id_slice < n - 1 ; ++id_slice )
    {
      m_Dogs_gpu.slices[ id_slice ] = m_ctx.createImage( octave.img_width , octave.img_height , system::gpu::OPENCL_IMAGE_CHANNEL_ORDER_R , system::gpu::OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;
    }
  }


  m_Dogs_gpu.octave_level = octave.octave_level ;
  m_Dogs_gpu.delta = octave.delta ;
  m_Dogs_gpu.sigmas = octave.sigmas ;
  m_Dogs_gpu.img_width = octave.img_width ;
  m_Dogs_gpu.img_height = octave.img_height ;

  const size_t offset_region[] = { 0 , 0 } ;
  const size_t region_size[] = { octave.img_width , octave.img_height } ;

  // Compute Dogs
  for ( int s = 0; s < m_Dogs.slices.size(); ++s )
  {
    bool ok = image::gpu::ImageSub( m_Dogs_gpu.slices[ s ] , octave.slices[s + 1] , octave.slices[s] , offset_region , region_size , m_ctx ) ;
    if( ! ok )
    {
      return false ;
    }
    ok = image::gpu::FromOpenCLImage( m_Dogs_gpu.slices[ s ] , offset_region , region_size , m_Dogs.slices[s] , m_ctx ) ;
    if( ! ok )
    {
      return false ;
    }
  }

  return true;
}


/**
* @brief Compute the 2D Hessian response of the DoG operator is computed via finite difference schemes
* @param key A Keypoint (the field edgeResp will be updated)
* @retval the Harris and Stephen Edge response value
*/
float SIFT_KeypointExtractorGPU::Compute_edge_response
(
  Keypoint & key
) const
{
  const int s = key.s;
  const int i = key.i; // i = id_row
  const int j = key.j; // j = id_col
  const Octave & octave = m_Dogs;
  const image::Image<float> & im = octave.slices[s];
  // Compute the 2d Hessian at pixel (i,j)
  const float hXX = im( j, i - 1 ) + im( j, i + 1 ) - 2.f * im( j, i );
  const float hYY = im( j + 1, i ) + im( j - 1, i ) - 2.f * im( j, i );
  const float hXY = ( ( im( j + 1, i + 1 ) - im( j - 1, i + 1 ) ) - ( ( im( j + 1, i - 1 ) - im( j - 1, i - 1 ) ) ) ) / 4.f;
  // Harris and Stephen Edge response
  key.edgeResp = Square( hXX + hYY ) / ( hXX * hYY - hXY * hXY );
  return key.edgeResp;
}

/**
* @brief Find discrete extrema position (position, scale) in the Dog domain
* @param[out] keypoints The list of found extrema as Keypoints
* @param percent Percentage applied on of the internal Edge threshold value
*/
void SIFT_KeypointExtractorGPU::Find_3d_discrete_extrema
(
  std::vector<Keypoint> & keypoints,
  float percent ) const
{
  const int ns = m_Dogs.slices.size();
  const float delta = m_Dogs.delta;
  const int h = m_Dogs.slices[0].Height();
  const int w = m_Dogs.slices[0].Width();

  // Loop through the slices of the image stack (one octave)
  for ( int s = 1; s < ns - 1; ++s )
  {
    for ( int id_row = 1; id_row < h - 1; ++id_row )
    {
      for ( int id_col = 1; id_col < w - 1; ++id_col )
      {
        const float pix_val = m_Dogs.slices[s]( id_row, id_col );
        if ( m_local_min_max_cpu.slices[ s - 1 ]( id_row , id_col ) > 0.f )
        {
          // if 3d discrete extrema, save a candidate keypoint
          Keypoint key;
          key.i = id_col;
          key.j = id_row;
          key.s = s;
          key.o = m_Dogs.octave_level;
          key.x = delta * id_col;
          key.y = delta * id_row;
          key.sigma = m_Dogs.sigmas[s];
          key.val = pix_val;
          keypoints.emplace_back( key );
        }
      }
    }
  }
  keypoints.shrink_to_fit();
}


/**
* @brief Refine the 3D location of a Keypoint using the local Hessian value (discrete to subpixel)
* @param stack The list of found extrema as Keypoints
* @param i Input discrete x location of the keypoint
* @param j Input discrete y location of the keypoint
* @param s Input scale of the keypoint
* @param di Refined subpixel x location
* @param dj Refined subpixel y location
* @param ds Refined scale value
* @param val Refined local intensity of the keypoint (Peak value)
* @param ofstMax Upper limit of variation of the refined parameters
* @retval true if the value have been refined
* @retval false if the value cannot be refined (inside the provide range)
*/
static inline bool Inverse_3D_Taylor_second_order_expansion
(
  const Octave & stack, // the dog stack
  int i, int j, int s,
  float *di, float *dj, float *ds, float *val,
  const float ofstMax
)
{
  float hXX, hXY, hXS, hYY, hYS, hSS;
  float gX, gY, gS;
  float ofstX, ofstY, ofstS, ofstVal;

  const image::Image<float> & slice = stack.slices[s];
  const image::Image<float> & sliceU = stack.slices[s + 1];
  const image::Image<float> & sliceD = stack.slices[s - 1];

  // Compute the 3d Hessian at pixel (i,j,s)  Finite difference scheme
  hXX = slice( j, i - 1 ) + slice( j, i + 1 ) - 2.f * slice( j, i );
  hYY = slice( j + 1, i ) + slice( j - 1, i ) - 2.f * slice( j, i );
  hSS = sliceU( j, i )  + sliceD( j, i )  - 2.f * slice( j, i );
  hXY = (  ( slice( j + 1, i + 1 ) - slice( j - 1, i + 1 ) )
           - ( slice( j + 1, i - 1 ) - slice( j - 1, i - 1 ) ) ) * 0.25f ;
  hXS = (  ( sliceU( j, i + 1 )  - sliceU( j, i - 1 ) )
           - ( sliceD( j, i + 1 )  - sliceD( j, i - 1 ) ) ) * 0.25f ;
  hYS = (  ( sliceU( j + 1, i )  - sliceU( j - 1, i ) )
           - ( sliceD( j + 1, i )  - sliceD( j - 1, i ) ) ) * 0.25f ;

  // Compute the 3d gradient at pixel (i,j,s)
  gX = ( slice( j, i + 1 ) - slice( j, i - 1 ) ) * 0.5f ;
  gY = ( slice( j + 1, i ) - slice( j - 1, i ) ) * 0.5f ;
  gS = ( sliceU( j, i )  - sliceD( j, i )  ) * 0.5f ;

  // Inverse the Hessian - Fitting a quadratic function
  Eigen::Matrix<float, 3, 3> A;
  Vec3f b;
  A << hXX, hXY, hXS, hXY, hYY, hYS, hXS, hYS, hSS;
  b << -gX, -gY, -gS;

  // solve for offset
  Eigen::FullPivLU<Eigen::Matrix<float, 3, 3>> lu_decomp( A );
  if ( !lu_decomp.isInvertible() )
  {
    return false;
  }

  const Vec3f dst = lu_decomp.solve( b );
  ofstX = dst( 0 );
  ofstY = dst( 1 );
  ofstS = dst( 2 );
  // Compute the DoG value offset
  ofstVal = ( gX * ofstX + gY * ofstY + gS * ofstS ) / 2.f;

  // output
  *di = ofstX;
  *dj = ofstY;
  *ds = ofstS;
  *val = slice( j, i ) + ofstVal;

  // return true is the quadratic model is consistent (to the given range)
  return std::abs( ofstX ) < ofstMax && std::abs( ofstY ) < ofstMax && std::abs( ofstS ) < ofstMax;
}


/**
* @brief Tell if a keypoint is close to the border according its scale
* @param key The list of found extrema as Keypoints
* @param w Slice image width
* @param h Slice image height
* @param lambda Scaling factor apply on the keypoint scale
* @retval true if the keypoint is in the [0+lambda*key.sigma; (w or h) - lambda*key.sigma] range
* @retval false if the keypoint is too close the image border
*/
static inline bool Border_Check( const Keypoint & key, int w, int h, float lambda = 1.f )
{
  const float x = key.x;
  const float y = key.y;
  const int octave = key.o;
  const float ratio = 1 << octave; //pow(2,p);
  const float sigma = key.sigma;
  const bool isIn = ( x - lambda * sigma > 0.0 ) && ( x + lambda * sigma < ( float )w * ratio )
                    && ( y - lambda * sigma > 0.0 ) && ( y + lambda * sigma < ( float )h * ratio );
  return isIn;
}



/**
* @brief Refine the keypoint position (location in space and scale), discard keypoints that cannot be refined.
* @param[in,out] key The list of refined keypoints
*/
void SIFT_KeypointExtractorGPU::Keypoints_refine_position
(
  std::vector<Keypoint> & keypoints
) const
{
  std::vector<Keypoint> kps;
  kps.reserve( keypoints.size() );

  const float ofstMax = 0.6f;

  // Ratio between two consecutive scales in the slice
  const float sigma_ratio = m_Dogs.sigmas[1] / m_Dogs.sigmas[0];
  const float edge_thres = Square( m_edge_threshold + 1 ) / m_edge_threshold;

  const Octave & octave = m_Dogs;
  const int w = octave.slices[0].Width();
  const int h = octave.slices[0].Height();
  const float delta  = octave.delta;

  for ( const auto & key : keypoints )
  {
    float val = key.val;

    int ic = key.i; // current discrete value of x coordinate - at each interpolation
    int jc = key.j; // current discrete value of y coordinate - at each interpolation
    int sc = key.s; // current discrete value of s coordinate - at each interpolation

    float ofstX = 0.0f;
    float ofstY = 0.0f;
    float ofstS = 0.0f;

    bool isConv = false;
    // While position cannot be refined and the number of refinement is not too much
    for ( int nIntrp = 0; !isConv && nIntrp < m_nb_refinement_step; ++nIntrp )
    {
      // Extrema interpolation via a quadratic function
      //   only if the detection is not too close to the border (so the discrete 3D Hessian is well defined)
      if ( 0 < ic &&  ic < ( w - 1 ) && 0 < jc && jc < ( h - 1 ) )
      {
        if ( Inverse_3D_Taylor_second_order_expansion( octave, ic, jc, sc, &ofstX, &ofstY, &ofstS, &val, ofstMax ) )
        {
          isConv = true;
        }
      }
      else
      {
        isConv = false;
      }
      if ( !isConv )
      {
        // let's explore the neighbourhood in
        // space...
        if ( ofstX > +ofstMax && ( ic + 1 ) < ( w - 1 ) )
        {
          ++ic;
        }
        if ( ofstX < -ofstMax && ( ic - 1 ) >  0    )
        {
          --ic;
        }
        if ( ofstY > +ofstMax && ( jc + 1 ) < ( h - 1 ) )
        {
          ++jc;
        }
        if ( ofstY < -ofstMax && ( jc - 1 ) >  0    )
        {
          --jc;
        }
        // ... and scale.
        /*
        if (ofstS > +ofstMax && (sc+1) < (ns-1)) {++sc;}
        if (ofstS < -ofstMax && (sc-1) >    0  ) {--sc;}
        */
      }
    }

    if ( isConv )
    {
      // Peak threshold check
      if ( std::abs( val ) > m_peak_threshold )
      {
        Keypoint kp = key;
        kp.x = ( ic + ofstX ) * delta;
        kp.y = ( jc + ofstY ) * delta;
        kp.i = ic;
        kp.j = jc;
        kp.s = sc;
        kp.sigma = octave.sigmas[sc] * pow( sigma_ratio, ofstS ); // logarithmic scale
        kp.val = val;
        // Edge check
        if ( Compute_edge_response( kp ) >= 0 && std::abs( kp.edgeResp ) <= edge_thres )
        {
          // Border check
          if ( Border_Check( kp, w, h ) )
          {
            kps.emplace_back( kp );
          }
        }
      }
    }
  }
  keypoints = std::move( kps );
  keypoints.shrink_to_fit();
}

// Find keypoints then refine it
void SIFT_KeypointExtractorGPU::Find_and_refine_keypoints
(
  std::vector<Keypoint> & keypoints
  const float percent ) const
{
  const float threshold = m_peak_threshold * percent;

  const size_t offset_region[] = { 0 , 0 } ;
  const size_t region_size[] = { m_Dogs_gpu.img_width , m_Dogs_gpu.img_height } ;

  // Compute min max
  cl_mem minMax = m_ctx.getTemporaryImage1( m_Dogs_gpu.img_width , m_Dogs_gpu.img_height , OPENCL_IMAGE_CHANNEL_ORDER_R , OPENCL_IMAGE_DATA_TYPE_FLOAT ) ;

  for ( int s = 1; s < ns - 1; ++s )
  {
    cl_mem prev_dog = m_Dogs_gpu.slices[ s - 1 ] ;
    cl_mem cur_dog = m_Dogs_gpu.slices[ s ] ;
    cl_mem next_dog = m_Dogs_gpu.slices[ s + 1 ] ;

    ImageLocalMaxima( minMax , prev_dog , cur_dog , next_dog , m_ctx , offset_region , region_size , threshold , m_ctx ) ;
  }
}



} // namespace gpu
} // namespace sift
} // namespace features
} // namespace openMVG
