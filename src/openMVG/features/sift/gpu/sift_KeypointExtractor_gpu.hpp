// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.
// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef OPENMVG_FEATURES_SIFT_GPU_SIFT_KEYPOINT_EXTRACTOR_GPU_HPP
#define OPENMVG_FEATURES_SIFT_GPU_SIFT_KEYPOINT_EXTRACTOR_GPU_HPP


/*

== Patent Warning and License =================================================

The SIFT method is patented

    [2] "Method and apparatus for identifying scale invariant features
      in an image."
        David G. Lowe
        Patent number: 6711293
        Filing date: Mar 6, 2000
        Issue date: Mar 23, 2004
        Application number: 09/519,89

 These source codes are made available for the exclusive aim of serving as
 scientific tool to verify the soundness and completeness of the algorithm
 description. Compilation, execution and redistribution of this file may
 violate patents rights in certain countries. The situation being different
 for every country and changing over time, it is your responsibility to
 determine which patent rights restrictions apply to you before you compile,
 use, modify, or redistribute this file. A patent lawyer is qualified to make
 this determination. If and only if they don't conflict with any patent terms,
 you can benefit from the following license terms attached to this file.

The implementation is based on

    [1] "Anatomy of the SIFT Method."
        I. Rey Otero  and  M. Delbracio
        Image Processing Online, 2013.
        http://www.ipol.im/pub/algo/rd_anatomy_sift/
*/

#include <vector>

#include "openMVG/features/feature.hpp"
#include "openMVG/features/sift/hierarchical_gaussian_scale_space.hpp"
#include "openMVG/features/sift/gpu/hierarchical_gaussian_scale_space_gpu.hpp"

#include "openMVG/features/sift/sift_keypoint.hpp"

#include "openMVG/image/image_container.hpp"

#include "openMVG/system/gpu/OpenCLContext.hpp"

namespace openMVG
{
namespace features
{
namespace sift
{
namespace gpu
{

struct SIFT_KeypointExtractorGPU
{
    /**
    * @brief SIFT_KeypointExtractor constructor
    * @param peak_threshold Threshold on DoG operator
    * @param edge_threshold Threshold on the ratio of principal curvatures
    * @param nb_refinement_step Maximum number of refinement step to find exact location of interest point
    * @param ctx OpenCL Context
    */
    SIFT_KeypointExtractorGPU
    (
      float peak_threshold,     // i.e => 0.04 / slices per octave
      float edge_threshold,     // i.e => 10
      int nb_refinement_step = 5 ,
      const openMVG::system::gpu::OpenCLContext & ctx = std::move( openMVG::system::gpu::OpenCLContext() )
    );

    /**
    * @brief Detect Scale Invariant points using Difference of Gaussians
    * @param octave A Gaussian octave
    * @param[out] keypoints The found Scale Invariant keypoint
    */
    void operator()( const openMVG::features::gpu::GPUOctave & octave , std::vector<Keypoint> & keypoints ) ;

  protected:

    /**
    * @brief Compute the Difference of Gaussians (Dogs) for a Gaussian octave
    * @param Octave The input Gaussian octave
    * @retval true If Dogs have been computed
    * @retval false if Dogs cannot be computed (less than 2 images)
    */
    bool ComputeDogs( const openMVG::features::gpu::GPUOctave & octave ) ;



    /**
    * @brief Compute the 2D Hessian response of the DoG operator is computed via finite difference schemes
    * @param key A Keypoint (the field edgeResp will be updated)
    * @retval the Harris and Stephen Edge response value
    */
    float Compute_edge_response
    (
      Keypoint & key
    ) const ;


    /**
    * @brief Find discrete extrema position (position, scale) in the Dog domain
    * @param[out] keypoints The list of found extrema as Keypoints
    * @param percent Percentage applied on of the internal Edge threshold value
    */
    void Find_3d_discrete_extrema
    (
      std::vector<Keypoint> & keypoints,
      float percent = 1.0f
    ) const ;


    /**
    * @brief Refine the keypoint position (location in space and scale), discard keypoints that cannot be refined.
    * @param[in,out] key The list of refined keypoints
    */
    void Keypoints_refine_position
    (
      std::vector<Keypoint> & keypoints
    ) const ;

  protected:

    system::gpu::OpenCLContext m_ctx ;

    Octave m_Dogs;

    // Keypoint detection parameters
    float m_peak_threshold;     // threshold on DoG operator
    float m_edge_threshold;    // threshold on the ratio of principal curvatures
    int m_nb_refinement_step; // Maximum number of refinement step to find exact location of interest point
};

} // namespace gpu
} // namespace sift
} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_SIFT_SIFT_KEYPOINT_EXTRACTOR_HPP
