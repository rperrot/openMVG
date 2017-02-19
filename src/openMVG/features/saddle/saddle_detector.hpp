// Copyright (c) 2017 Romuald Perrot.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_SADDLE_SADDLE_DETECTOR_HPP
#define OPENMVG_FEATURES_SADDLE_SADDLE_DETECTOR_HPP

#include "openMVG/features/feature.hpp"
#include "openMVG/image/image.hpp"

namespace openMVG
{
namespace features
{
  /**
  * implementation of paper : "In the Saddle: Chasing Fast and Repeatable Features""
  * Javier Aldana-Iuit, Dmytro Mishkin, Ondrej Chum and Jiŕı Matas
  */
  class SaddleDetector
  {
  public:
    /**
      * @brief Initialize detector 
      * @param nb_level Number of pyramid level 
      * @param scaling Scale between two pyramid level 
      * @brief epsilon sensibility for classification af saddle points 
      */
    SaddleDetector( const int nb_level = 6, const double scaling = 1.3, const int epsilon = 5 );

    /**
      * @brief Detect saddle points in the given image 
      * @param ima Image image in which points are extracted
      * @param[out] regions Detected points 
      * @note only scale is computed, not orientation 
      */
    void detect( const image::Image<unsigned char> &ima, std::vector<SIOPointFeature> &regions ) const;

  private:
    /**
    * @brief compute points that pass alternating inner test and outer tests 
    * @param ima Image image in which points are extracted
    * @param[out] pts The points position 
    * @param[out] Ips The interpolated feature intensity (at the saddle point)
    */
    void alternatingInnerOuter( const image::Image<unsigned char> &ima,
                                std::vector<PointFeature> &pts,
                                std::vector<unsigned char> &Ips ) const;

    /**
    * @brief Compute non max suppression and final point localisation 
    * @param ima Image 
    * @param putative Putative points to filter 
    * @param putativeIps Putative points intensity to filter 
    * @param[out] filteredPts Final points after filtering and position assignment 
    */
    void nonMaxSuppression( const image::Image<unsigned char> &ima,
                            const std::vector<PointFeature> &putatives,
                            const std::vector<unsigned char> &putativeIps,
                            std::vector<PointFeature> &filteredPts ) const;

    /**
    * @brief Compute new pyramid level from current one 
    * @param[in,out] slice Current and output slice 
    */
    void prepareSlice( image::Image<unsigned char> &slice ) const;

    /// Number of pyramid level
    int m_nb_level;
    /// Scale between two consecutives pyramid level
    double m_scaling;
    /// Espilon factor to decide if points is d/s/l
    /* d : intensity above central pixel 
     * s : intensity almost same as central pixel 
     * l : intensity lower than central pixel 
     */
    int m_epsilon;
  };

} // namespace features
} // namespace openMVG

#endif