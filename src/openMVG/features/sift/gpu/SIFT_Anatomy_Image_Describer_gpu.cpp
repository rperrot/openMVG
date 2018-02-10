// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.
// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "SIFT_Anatomy_Image_Describer_gpu.hpp"

#include "openMVG/features/sift/gpu/hierarchical_gaussian_scale_space_gpu.hpp"
#include "openMVG/features/sift/gpu/sift_keypointExtractor_gpu.hpp"

#include "openMVG/features/sift/sift_keypoint.hpp"
#include "openMVG/features/sift/sift_KeypointExtractor.hpp"
#include "openMVG/features/sift/sift_DescriptorExtractor.hpp"


namespace openMVG
{
namespace features
{
namespace gpu
{

SIFT_Anatomy_Image_describerGPU::Params::Params( int first_octave ,
    int num_octaves ,
    int num_scales ,
    float edge_threshold ,
    float peak_threshold ,
    bool root_sift )
  : first_octave_( first_octave ),
    num_octaves_( num_octaves ),
    num_scales_( num_scales ),
    edge_threshold_( edge_threshold ),
    peak_threshold_( peak_threshold ),
    root_sift_( root_sift )
{

}

SIFT_Anatomy_Image_describerGPU::SIFT_Anatomy_Image_describerGPU(
  const Params & params )
  : Image_describer(),
    params_( params )
{

}

bool SIFT_Anatomy_Image_describerGPU::Set_configuration_preset( EDESCRIBER_PRESET preset )
{
  switch ( preset )
  {
    case NORMAL_PRESET:
      params_.peak_threshold_ = 0.04f;
      break;
    case HIGH_PRESET:
      params_.peak_threshold_ = 0.01f;
      break;
    case ULTRA_PRESET:
      params_.peak_threshold_ = 0.01f;
      params_.first_octave_ = -1;
      break;
    default:
      return false;
  }
  return true;
}

/**
@brief Detect regions on the image and compute their attributes (description)
@param image Image.
@param mask 8-bit gray image for keypoint filtering (optional).
   Non-zero values depict the region of interest.
@return regions The detected regions and attributes (the caller must delete the allocated data)
*/
std::unique_ptr<SIFT_Anatomy_Image_describerGPU::Regions_type> SIFT_Anatomy_Image_describerGPU::Describe_SIFT_AnatomyGPU(
  const image::Image<unsigned char>& image,
  const image::Image<unsigned char>* mask )
{
  auto regions = std::unique_ptr<Regions_type>( new Regions_type );

  if ( image.size() == 0 )
  {
    return regions;
  }

  // Convert to float in range [0;1]
  const image::Image<float> If( image.GetMat().cast<float>() / 255.0f );

  // compute sift keypoints
  {
    const int supplementary_images = 3;
    // => in order to ensure each gaussian slice is used in the process 3 extra images are required:
    // +1 for dog computation
    // +2 for 3d discrete extrema definition

    system::gpu::OpenCLContext ctx ;

    HierarchicalGaussianScaleSpaceGPU octave_gen(
      params_.num_octaves_,
      params_.num_scales_,
      ( params_.first_octave_ == -1 )
      ? GaussianScaleSpaceParams( 1.6f / 2.0f, 1.0f / 2.0f, 0.5f, supplementary_images )
      : GaussianScaleSpaceParams( 1.6f, 1.0f, 0.5f, supplementary_images ) ,
      ctx );
    octave_gen.SetImage( If );

    std::vector<sift::Keypoint> keypoints;
    keypoints.reserve( 5000 );

    GPUOctave gpu_octave ;
    Octave cpu_octave;

    // Find Keypoints
    sift::gpu::SIFT_KeypointExtractorGPU keypointDetector(
      params_.peak_threshold_ / octave_gen.NbSlice(),
      params_.edge_threshold_ ,
      5 ,
      ctx );

    while ( octave_gen.NextOctave( gpu_octave ) )
    {
      std::vector<sift::Keypoint> keys;

      // Convert to cpu octave to perform latter computations
      gpu_octave.convertToCPUOctave( cpu_octave , ctx ) ;

      keypointDetector( gpu_octave, keys );


      // Find Keypoints orientation and compute their description
      sift::Sift_DescriptorExtractor descriptorExtractor;
      descriptorExtractor( cpu_octave, keys );

      // Concatenate the found keypoints
      std::move( keys.begin(), keys.end(), std::back_inserter( keypoints ) );
    }
    for ( const auto & k : keypoints )
    {
      // Feature masking
      if ( mask )
      {
        const image::Image<unsigned char> & maskIma = *mask;
        if ( maskIma( k.y, k.x ) == 0 )
        {
          continue;
        }
      }

      Descriptor<unsigned char, 128> descriptor;
      descriptor << ( k.descr.cast<unsigned char>() );
      {
        regions->Descriptors().emplace_back( descriptor );
        regions->Features().emplace_back( k.x, k.y, k.sigma, k.theta );
      }
    }
  }
  return regions;
}

std::unique_ptr<Regions> SIFT_Anatomy_Image_describerGPU::Allocate() const
{
  return std::unique_ptr<Regions_type>( new Regions_type );
}

std::unique_ptr<Regions> SIFT_Anatomy_Image_describerGPU::Describe(
  const image::Image<unsigned char>& image,
  const image::Image<unsigned char>* mask )
{
  return Describe_SIFT_AnatomyGPU( image, mask );
}


} // namespace gpu
} // namespace features
} // namespace openMVG

