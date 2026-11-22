// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_SFM_SFM_FEATURES_PROVIDER_HPP
#define OPENMVG_SFM_SFM_FEATURES_PROVIDER_HPP

#include <memory>
#include <string>
#include <vector>

#include "openMVG/features/feature.hpp"
#include "openMVG/features/feature_container.hpp"
#include "openMVG/features/regions.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/system/logger.hpp"
#include "openMVG/system/loggerprogress.hpp"
#include "openMVG/types.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

namespace openMVG {
namespace sfm {

/// Abstract PointFeature provider (read some feature and store them as PointFeature).
/// Allow to load and return the features related to a view
struct Features_Provider
{
  /// PointFeature array per ViewId of the considered SfM_Data container
  Hash_Map<IndexT, features::PointFeatures> feats_per_view;
  /// Optional per-feature dominant orientations (radians) and detection scales (pixels)
  /// keyed by ViewId; populated only when the loaded Regions type exposes them (e.g. SIFT/AKAZE).
  Hash_Map<IndexT, std::vector<float>> orientations_per_view;
  Hash_Map<IndexT, std::vector<float>> scales_per_view;

  virtual ~Features_Provider() = default;

  virtual bool load(
    const SfM_Data & sfm_data,
    const std::string & feat_directory,
    std::unique_ptr<features::Regions>& region_type)
  {
    system::LoggerProgress my_progress_bar(sfm_data.GetViews().size(), "- Features Loading -");
    // Read for each view the corresponding features and store them as PointFeatures
    bool bContinue = true;
#ifdef OPENMVG_USE_OPENMP
    #pragma omp parallel
#endif
    for (Views::const_iterator iter = sfm_data.GetViews().begin();
      iter != sfm_data.GetViews().end() && bContinue; ++iter)
    {
#ifdef OPENMVG_USE_OPENMP
    #pragma omp single nowait
#endif
      {
        const std::string sImageName = stlplus::create_filespec(sfm_data.s_root_path, iter->second->s_Img_path);
        const std::string basename = stlplus::basename_part(sImageName);
        const std::string featFile = stlplus::create_filespec(feat_directory, basename, ".feat");

        std::unique_ptr<features::Regions> regions(region_type->EmptyClone());
        if (!stlplus::file_exists(featFile) || !regions->LoadFeatures(featFile))
        {
          OPENMVG_LOG_ERROR << "Invalid feature files for the view: " << sImageName;
#ifdef OPENMVG_USE_OPENMP
      #pragma omp critical
#endif
          bContinue = false;
        }
        // Extract positions and optional scale/orientation outside the OpenMP critical section
        auto positions = regions->GetRegionsPositions();
        auto orientations = regions->GetRegionsOrientations();
        auto scales = regions->GetRegionsScales();
#ifdef OPENMVG_USE_OPENMP
      #pragma omp critical
#endif
        {
          // Save loaded features as PointFeature plus optional orientation & scale arrays
          const IndexT view_id = iter->second->id_view;
          feats_per_view[view_id] = std::move(positions);
          if (!orientations.empty())
            orientations_per_view[view_id] = std::move(orientations);
          if (!scales.empty())
            scales_per_view[view_id] = std::move(scales);
        }
        ++my_progress_bar;
      }
    }
    return bContinue;
  }

  /// Return the PointFeatures belonging to the View, if the view does not exist
  ///  it returns an empty PointFeature array.
  const features::PointFeatures & getFeatures(const IndexT & id_view) const
  {
    // Have an empty feature set in order to deal with non existing view_id
    static const features::PointFeatures emptyFeats = features::PointFeatures();

    Hash_Map<IndexT, features::PointFeatures>::const_iterator it = feats_per_view.find(id_view);
    if (it != feats_per_view.end())
      return it->second;
    else
      return emptyFeats;
  }

  /// Return the orientation (radians) of a feature if available, or 0.0f otherwise.
  float getFeatureOrientation(const IndexT id_view, const size_t id_feat) const
  {
    const auto it = orientations_per_view.find(id_view);
    if (it != orientations_per_view.end() && id_feat < it->second.size())
      return it->second[id_feat];
    return 0.0f;
  }

  /// Return the scale (pixels) of a feature if available, or 0.0f otherwise.
  float getFeatureScale(const IndexT id_view, const size_t id_feat) const
  {
    const auto it = scales_per_view.find(id_view);
    if (it != scales_per_view.end() && id_feat < it->second.size())
      return it->second[id_feat];
    return 0.0f;
  }
}; // Features_Provider

} // namespace sfm
} // namespace openMVG

#endif // OPENMVG_SFM_SFM_FEATURES_PROVIDER_HPP
