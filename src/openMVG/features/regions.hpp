// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_FEATURES_REGIONS_HPP
#define OPENMVG_FEATURES_REGIONS_HPP

#include <cmath>
#include <string>
#include <type_traits>
#include <vector>
#include <openMVG/features/feature.hpp>
#include <openMVG/features/feature_container.hpp>
#include <openMVG/numeric/eigen_alias_definition.hpp>

namespace openMVG {
namespace features {

namespace detail {

// C++11 SFINAE helpers (rather than C++17 `if constexpr`) are intentionally used here
// because public region headers are also included by GUI/sample targets compiled in C++11/14.

/**
 * @brief Extract dominant feature orientations (in radians) when the feature type
 *        exposes `orientation()` (e.g. `SIOPointFeature`, `AffinePointFeature`).
 */
template <typename FeatT>
inline typename std::enable_if<
  std::is_base_of<SIOPointFeature, FeatT>::value ||
    std::is_base_of<AffinePointFeature, FeatT>::value,
  std::vector<float>>::type
ExtractRegionsOrientations(const std::vector<FeatT>& feats)
{
  std::vector<float> orientations;
  orientations.reserve(feats.size());
  for (const auto& feat : feats)
    orientations.push_back(feat.orientation());
  return orientations;
}

/**
 * @brief Fallback orientation extractor for unoriented point types (e.g. `PointFeature`).
 */
template <typename FeatT>
inline typename std::enable_if<
  !std::is_base_of<SIOPointFeature, FeatT>::value &&
    !std::is_base_of<AffinePointFeature, FeatT>::value,
  std::vector<float>>::type
ExtractRegionsOrientations(const std::vector<FeatT>&)
{
  return {};
}

/**
 * @brief Extract characteristic feature scales (in pixels) for `SIOPointFeature` regions.
 */
template <typename FeatT>
inline typename std::enable_if<
  std::is_base_of<SIOPointFeature, FeatT>::value,
  std::vector<float>>::type
ExtractRegionsScales(const std::vector<FeatT>& feats)
{
  std::vector<float> scales;
  scales.reserve(feats.size());
  for (const auto& feat : feats)
    scales.push_back(feat.scale());
  return scales;
}

/**
 * @brief Extract characteristic isotropic scale for `AffinePointFeature` via the geometric
 *        mean of its ellipse semi-axes: $s = \sqrt{\ell_1 \ell_2}$ (area-equivalent radius).
 */
template <typename FeatT>
inline typename std::enable_if<
  !std::is_base_of<SIOPointFeature, FeatT>::value &&
    std::is_base_of<AffinePointFeature, FeatT>::value,
  std::vector<float>>::type
ExtractRegionsScales(const std::vector<FeatT>& feats)
{
  std::vector<float> scales;
  scales.reserve(feats.size());
  for (const auto& feat : feats)
    scales.push_back(std::sqrt(feat.l1() * feat.l2()));
  return scales;
}

/**
 * @brief Fallback scale extractor for scale-free point types (e.g. `PointFeature`).
 */
template <typename FeatT>
inline typename std::enable_if<
  !std::is_base_of<SIOPointFeature, FeatT>::value &&
    !std::is_base_of<AffinePointFeature, FeatT>::value,
  std::vector<float>>::type
ExtractRegionsScales(const std::vector<FeatT>&)
{
  return {};
}

} // namespace detail

/// Describe an image a set of regions (position, ...) + attributes
/// Each region is described by a set of attributes (descriptor)
class Regions
{
public:

  virtual ~Regions() = default;

  //--
  // IO - one file for region features, one file for region descriptors
  //--

  virtual bool Load(
    const std::string& sfileNameFeats,
    const std::string& sfileNameDescs) = 0;

  virtual bool Save(
    const std::string& sfileNameFeats,
    const std::string& sfileNameDescs) const = 0;

  virtual bool LoadFeatures(
    const std::string& sfileNameFeats) = 0;

  /// Save only feature geometry/attributes (`.feat`) while leaving descriptors untouched
  virtual bool SaveFeatures(
    const std::string& sfileNameFeats) const = 0;

  //--
  //- Basic description of a descriptor [Type, Length]
  //--
  virtual bool IsScalar() const = 0;
  virtual bool IsBinary() const = 0;

  /// basis element used for description
  virtual std::string Type_id() const = 0;
  virtual size_t DescriptorLength() const = 0;
  //--

  //-- Assume that a region can always be represented at least by a 2D positions
  virtual PointFeatures GetRegionsPositions() const = 0;
  virtual Vec2 GetRegionPosition(size_t i) const = 0;
  /// Update the 2D pixel coordinates $(x, y)$ of region @p i in place
  virtual void SetRegionPosition(size_t i, float x, float y) = 0;

  //-- Optional feature orientation (radians, $\theta \in [0, 2\pi)$) and scale (pixels) getters.
  //-- Return empty vectors when the underlying feature type does not carry these attributes.
  virtual std::vector<float> GetRegionsOrientations() const { return {}; }
  virtual std::vector<float> GetRegionsScales() const { return {}; }

  /// Return the number of defined regions
  virtual size_t RegionCount() const = 0;

  /// Return a pointer to the first value of the descriptor array
  // Used to avoid complex template imbrication
  virtual const void * DescriptorRawData() const = 0;

  /// Return the squared distance between two descriptors
  // A default metric is used according the descriptor type:
  // - Scalar: SquaredL2,
  // - Binary: SquaredHamming
  virtual double SquaredDescriptorDistance(
    size_t i,
    const Regions *,
    size_t j) const = 0;

  /// Add the Inth region to another Region container
  virtual void CopyRegion(size_t i, Regions *) const = 0;

  /// Sort the regions according the scale of the feature
  /// If the regions are not Scale invariant nothing is done and false is returned
  /// keep_count is used to save feature from largest scale to lower (-1 keep everything)
  virtual bool SortAndSelectByRegionScale(int keep_count = -1) = 0;

  virtual Regions * EmptyClone() const = 0;

};

std::unique_ptr<features::Regions> Init_region_type_from_file
(
  const std::string & sImage_describer_file
);

} // namespace features
} // namespace openMVG

#endif // OPENMVG_FEATURES_REGIONS_HPP
