// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "hierarchical_gaussian_scale_space_gpu.hpp"

#include "openMVG/image/gpu/image_gpu_interface.hpp"
#include "openMVG/image/gpu/image_gpu_filtering.hpp"
#include "openMVG/image/gpu/image_gpu_resampling.hpp"


namespace openMVG
{
namespace features
{
namespace gpu
{

GPUOctave::~GPUOctave( void )
{
  for( auto & it : slices )
  {
    clReleaseMemObject( it ) ;
  }
}

/**
 * @brief Convert GPU octave to a CPU one
 * @param[out] cpu_octate The octave
 * @param ctx OpenCL Context
 */
void GPUOctave::convertToCPUOctave( Octave & cpu_octave , system::gpu::OpenCLContext & ctx )
{
  /*
  int octave_level;                   // the octave level
  float delta;                        // sampling rate in this octave
  std::vector< float > sigmas;        // sigma values
  std::vector< cl_mem > slices;       // octave slice (from fine to coarse)
   */
  cpu_octave.octave_level = octave_level ;
  cpu_octave.delta = delta ;
  cpu_octave.sigmas = sigmas ;
  cpu_octave.slices.resize( slices.size() ) ;
  for( size_t id_slice = 0 ; id_slice < slices.size() ; ++id_slice )
  {
    image::gpu::FromOpenCLImage( slices[ id_slice ] , cpu_octave.slices[ id_slice ] , ctx ) ;
  }
}



/**
* @brief HierarchicalGaussianScaleSpace constructor
* @param nb_octave Number of Octave
* @param nb_slice Number of slice per octave
* @param params Parameters of the Gaussian scale space
*/
HierarchicalGaussianScaleSpaceGPU::HierarchicalGaussianScaleSpaceGPU(
  const int nb_octave ,
  const int nb_slice ,
  const GaussianScaleSpaceParams & params ,
  const openMVG::system::gpu::OpenCLContext & ctx )
  : Octaver<GPUOctave>( nb_octave, nb_slice ),
    m_params( params ),
    m_cur_octave_id( 0 ),
    m_ctx( ctx )
{

}


/**
* @brief Set Initial image and update nb_octave if necessary
* @param img Input image
*/
void HierarchicalGaussianScaleSpaceGPU::SetImage( const image::Image<float> & img )
{
  cl_mem base_img = image::gpu::ToOpenCLImage( img , m_ctx ) ;

  const double sigma_extra =
    sqrt( Square( m_params.sigma_min ) - Square( m_params.sigma_in ) ) / m_params.delta_min;
  if ( m_params.delta_min == 1.0f )
  {
    m_cur_base_octave_image = image::gpu::ImageGaussianFilter( base_img , sigma_extra , m_ctx ) ;
    //    image::ImageGaussianFilter( img, sigma_extra, m_cur_base_octave_image );
  }
  else  // delta_min == 1
  {
    if ( m_params.delta_min == 0.5f )
    {
      cl_mem tmp = image::gpu::ImageUpsample( base_img , m_ctx ) ;
      m_cur_base_octave_image = image::gpu::ImageGaussianFilter( tmp , sigma_extra , m_ctx ) ;
      clReleaseMemObject( tmp ) ;
      //      image::Image<float> tmp;
      // ImageUpsample( img, tmp );
      // image::ImageGaussianFilter( tmp, sigma_extra, m_cur_base_octave_image );
    }
    else
    {
      std::cerr
          << "Upsampling or downsampling with delta equal to: "
          << m_params.delta_min << " is not yet implemented" << std::endl;
    }
  }
  clReleaseMemObject( base_img ) ;

  size_t width ;
  cl_int err = clGetImageInfo( m_cur_base_octave_image , CL_IMAGE_WIDTH , sizeof( size_t ) , &width , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return ; // false ;
  }
  size_t height ;
  err = clGetImageInfo( m_cur_base_octave_image , CL_IMAGE_HEIGHT , sizeof( size_t ) , &height , nullptr ) ;
  if( err != CL_SUCCESS )
  {
    return ; // false ;
  }



  //-- Limit the size of the last octave to be at least 32x32 pixels
  const int nbOctaveMax = std::ceil( std::log2( std::min( width , height ) / 32 ) );
  m_nb_octave = std::min( m_nb_octave, nbOctaveMax );
}


/**
* @brief Compute a full octave
* @param[out] oct Computed octave
* @retval true If an octave have been computed
* @retval false if no octave have been computed (process ended)
*/
bool HierarchicalGaussianScaleSpaceGPU::NextOctave( GPUOctave & octave )
{
  if ( m_cur_octave_id >= m_nb_octave )
  {
    return false;
  }
  else
  {
    if( octave.slices.size() > 0 )
    {
      for( auto & it : octave.slices )
      {
        clReleaseMemObject( it ) ;
      }
    }

    {
      octave.octave_level = m_cur_octave_id;
    }
    if ( m_cur_octave_id == 0 )
    {
      octave.delta = m_params.delta_min;
    }
    else
    {
      octave.delta *= 2.0f;
    }


    // init the "blur"/sigma scale spaces values
    octave.slices.resize( m_nb_slice + m_params.supplementary_levels );
    octave.sigmas.resize( m_nb_slice + m_params.supplementary_levels );
    for ( int s = 0; s < m_nb_slice  + m_params.supplementary_levels; ++s )
    {
      octave.sigmas[s] =
        octave.delta / m_params.delta_min * m_params.sigma_min * pow( 2.0, ( float )s / ( float )m_nb_slice );
    }

    // Build the octave iteratively
    octave.slices[0] = m_cur_base_octave_image;
    for ( int s = 1; s < octave.sigmas.size(); ++s )
    {
      // Iterative blurring the previous image
      cl_mem & im_prev = octave.slices[s - 1];
      cl_mem & im_next = octave.slices[s];
      const double sig_prev = octave.sigmas[s - 1];
      const double sig_next = octave.sigmas[s];
      const double sigma_extra = sqrt( Square( sig_next ) - Square( sig_prev ) ) / octave.delta;

      im_next = image::gpu::ImageGaussianFilter( im_prev , sigma_extra , m_ctx ) ;
      //      image::ImageGaussianFilter( im_prev, sigma_extra, im_next );
    }
    /*
    // Debug: Export DoG scale space on disk
    for (int s = 0; s < octave.sigmas.size(); ++s)
    {
      std::stringstream os;
      os << "DoG_out_00" << m_cur_octave_id << "_s" << "00" << s << ".png";
      image::WriteImage(os.str().c_str(), octave.slices[s]);
    }
    */

    // Prepare for next octave computation -> Decimation
    ++m_cur_octave_id;
    if ( m_cur_octave_id < m_nb_octave )
    {
      // Decimate => sigma * 2 for the next iteration
      const int index = ( m_params.supplementary_levels == 0 ) ? 1 : m_params.supplementary_levels;
      m_cur_base_octave_image = image::gpu::ImageDecimate( octave.slices[octave.sigmas.size() - index] , m_ctx ) ;
      // Note : it's not necessary to clear the m_cur_base_octave_image image since it's affected to the
      // first slice and will be deleted with the octave

      //      ImageDecimate( octave.slices[octave.sigmas.size() - index], m_cur_base_octave_image );
    }
    return true;
  }
}


} // namespace gpu
} // namespace features
} // namespace openMVG
