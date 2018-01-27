// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SIFT_GPU_HIERARCHICAL_GAUSSIAN_SCALE_SPACE_GPU_HPP
#define OPENMVG_FEATURES_SIFT_GPU_HIERARCHICAL_GAUSSIAN_SCALE_SPACE_GPU_HPP

#include "openMVG/features/sift/octaver.hpp"
#include "openMVG/features/sift/hierarchical_gaussian_scale_space.hpp"
#include "openMVG/system/gpu/OpenCLContext.hpp"

#include <vector>

namespace openMVG
{
namespace features
{
namespace gpu
{

struct GPUOctave
{
  int octave_level;                   // the octave level
  float delta;                        // sampling rate in this octave
  std::vector< float > sigmas;        // sigma values
  std::vector< cl_mem > slices;       // octave slice (from fine to coarse)

  /**
   * @brief Convert GPU octave to a CPU one
   * @param[out] cpu_octate The octave
   * @param ctx OpenCL Context
   */
  void convertToCPUOctave( Octave & cpu_octave , system::gpu::OpenCLContext & ctx ) ;

  ~GPUOctave( void ) ;
};



struct HierarchicalGaussianScaleSpaceGPU : public Octaver<GPUOctave>
{
    /**
    * @brief HierarchicalGaussianScaleSpace constructor
    * @param nb_octave Number of Octave
    * @param nb_slice Number of slice per octave
    * @param params Parameters of the Gaussian scale space
    * @param ctx OpenCL Context
    */
    HierarchicalGaussianScaleSpaceGPU(
      const int nb_octave = 6,
      const int nb_slice = 3,
      const GaussianScaleSpaceParams & params =
        std::move( GaussianScaleSpaceParams() ) ,
      const openMVG::system::gpu::OpenCLContext & ctx = std::move( openMVG::system::gpu::OpenCLContext() )
    ) ;

    /**
    * @brief Set Initial image and update nb_octave if necessary
    * @param img Input image
    */
    void SetImage( const image::Image<float> & img ) override ;

    /**
    * @brief Compute a full octave
    * @param[out] oct Computed octave
    * @retval true If an octave have been computed
    * @retval false if no octave have been computed (process ended)
    */
    bool NextOctave( GPUOctave & octave ) override ;


  protected:

    openMVG::system::gpu::OpenCLContext m_ctx ;

    GaussianScaleSpaceParams m_params;  // The Gaussian scale space parameters
    cl_mem m_cur_base_octave_image; // The image that will be used to generate the next octave
    int m_cur_octave_id; // The current Octave id [0 -> Octaver::m_nb_octave]
};

} // namespace gpu
} // namespace features
} // namespace openMVG

#endif