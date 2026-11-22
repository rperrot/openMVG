// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/**
 * @file multi_view_lsm_refiner_test.cpp
 * @brief Unit tests for `MultiViewLSMRefiner` covering affine convergence, partial occlusion,
 *        atomic rejection gates, 1D-edge aperture fallback, and disk `.feat` round-trip I/O.
 */

#include "openMVG/features/feature.hpp"
#include "openMVG/features/regions_factory.hpp"
#include "openMVG/image/image_container.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/sfm/pipelines/multi_view_lsm_refiner.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_view.hpp"
#include "openMVG/tracks/tracks.hpp"

#include "testing/testing.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace openMVG;
using namespace openMVG::features;
using namespace openMVG::image;
using namespace openMVG::sfm;
using namespace openMVG::tracks;

namespace {

/**
 * @brief Continuous analytical 2D texture evaluated directly at sub-pixel coordinates $(u, v)$.
 *
 * Combines non-collinear sinusoids (frequencies $\le 0.9\text{ rad/px}$, well below Nyquist)
 * with a localized Gaussian-modulated Gabor peak so the patch is non-periodic and non-isotropic
 * across a $13 \times 13$ optimization window. Evaluating analytically avoids resampling bias in GT.
 */
double EvalRichTexture(const double u, const double v)
{
  return 128.0
    + 32.0 * std::cos(0.45 * u + 0.20 * v)
    + 26.0 * std::sin(0.25 * u - 0.55 * v + 0.4)
    + 20.0 * std::cos(0.75 * u + 0.65 * v - 0.7)
    + 18.0 * std::exp(-(u * u + v * v) / 18.0) * std::cos(0.9 * u - 0.3 * v);
}

/**
 * @brief Synthesize a grayscale image by inverse-warping `EvalRichTexture` through $Q_{\text{true}}^{-1}$.
 *
 * Renders $I_i(x, y) = \operatorname{clamp}\big(\operatorname{round}(g_i F(Q_{\text{true}}^{-1}(\mathbf{x} - \mathbf{x}_i^{\text{gt}})) + o_i + \eta)\big)$,
 * so evaluating $I_i$ along the forward warp $W_i(\mathbf{u}) = \mathbf{x}_i^{\text{gt}} + Q_{\text{true}} \mathbf{u}$
 * recovers $g_i F(\mathbf{u}) + o_i$.
 */
Image<unsigned char> RenderWarpedPatchImage(
  const int width,
  const int height,
  const Eigen::Vector2d& x_gt,
  const Eigen::Matrix2d& Q_true,
  const double gain,
  const double offset,
  const double noise_sigma,
  const uint32_t seed)
{
  Image<unsigned char> img(width, height);
  const Eigen::Matrix2d Q_inv = Q_true.inverse();
  std::mt19937 rng(seed);
  std::normal_distribution<double> noise_dist(0.0, noise_sigma);

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const Eigen::Vector2d uv = Q_inv * (Eigen::Vector2d(x, y) - x_gt);
      double val = gain * EvalRichTexture(uv.x(), uv.y()) + offset;
      if (noise_sigma > 0.0)
        val += noise_dist(rng);
      val = std::min(255.0, std::max(0.0, std::round(val)));
      img(y, x) = static_cast<unsigned char>(val);
    }
  }
  return img;
}

/**
 * @brief Synthesize a pure 1D step edge oriented with normal $\mathbf{n} = (\cos\theta, \sin\theta)^T$:
 *        $I(x, y) = g (128 + 60 \tanh(0.75\, \mathbf{n}^T (\mathbf{x} - \mathbf{x}^{\text{gt}}))) + o$.
 * Translation along the tangent direction leaves intensities completely invariant (aperture problem).
 */
Image<unsigned char> Render1DEdgeImage(
  const int width,
  const int height,
  const Eigen::Vector2d& x_gt,
  const double theta,
  const double gain,
  const double offset)
{
  Image<unsigned char> img(width, height);
  const double c = std::cos(theta);
  const double s = std::sin(theta);

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const double dx = static_cast<double>(x) - x_gt.x();
      const double dy = static_cast<double>(y) - x_gt.y();
      const double u_perp = c * dx + s * dy;
      const double val = std::min(
        255.0,
        std::max(0.0, std::round(gain * (128.0 + 60.0 * std::tanh(0.75 * u_perp)) + offset)));
      img(y, x) = static_cast<unsigned char>(val);
    }
  }
  return img;
}

} // namespace

/**
 * @brief Verify sub-pixel (< 0.05 px) multi-view convergence under full 6-DoF oblique warps,
 *        arbitrary rotations, scale ratios, linear exposure changes, and Gaussian noise.
 */
TEST(MultiViewLSMRefiner, SyntheticAffineMultiViewConvergence)
{
  const int W = 128;
  const int H = 128;
  const int M = 8;

  const double thetas[M] = {0.0, 0.55, 1.15, 2.05, 3.14, 3.90, 4.75, 5.60};
  const double scales[M] = {2.0, 2.4, 1.7, 2.2, 1.85, 2.5, 2.05, 1.95};
  // Median of scales sorted: {1.7, 1.85, 1.95, 2.0, 2.05, 2.2, 2.4, 2.5} -> 2.025
  const double s_ref = 2.025;

  // Residual affine matrices simulating oblique 35-50 degree views (det in [0.7, 1.35])
  std::vector<Eigen::Matrix2d> M_true(M);
  M_true[0] << 1.00, 0.00, 0.00, 1.00;
  M_true[1] << 0.85, 0.08, -0.04, 1.02;
  M_true[2] << 1.15, -0.06, 0.05, 0.92;
  M_true[3] << 0.78, 0.10, 0.04, 1.05;
  M_true[4] << 1.12, 0.05, -0.07, 1.08;
  M_true[5] << 0.88, -0.09, 0.06, 0.90;
  M_true[6] << 1.08, 0.04, 0.05, 0.82;
  M_true[7] << 0.92, -0.05, -0.06, 1.15;

  // Balanced initial sub-pixel/pixel perturbations up to ~1.5 px (median ~ 0)
  const Eigen::Vector2d init_offsets[M] = {
    { 1.10, -0.60},
    {-1.10,  0.60},
    { 0.95,  0.90},
    {-0.95, -0.90},
    { 1.30,  0.30},
    {-1.30, -0.30},
    {-0.50,  1.20},
    { 0.50, -1.20}
  };

  const double gains[M]   = {1.00, 0.85, 1.20, 0.92, 1.10, 0.80, 1.18, 0.95};
  const double offsets[M] = { 0.0, 12.0, -10.0,  6.0, -14.0, 15.0, -6.0,  8.0};

  Hash_Map<IndexT, Image<unsigned char>> images;
  Features_Provider feats_provider;
  STLMAPTracks map_tracks;
  std::vector<Eigen::Vector2d> x_gt(M);

  for (int i = 0; i < M; ++i)
  {
    x_gt[i] = Eigen::Vector2d(64.23 + 0.1 * i, 63.77 - 0.08 * i);
    const double alpha = scales[i] / s_ref;
    Eigen::Matrix2d R;
    R << std::cos(thetas[i]), -std::sin(thetas[i]),
         std::sin(thetas[i]),  std::cos(thetas[i]);
    const Eigen::Matrix2d Q = alpha * R * M_true[i];

    images[static_cast<IndexT>(i)] = RenderWarpedPatchImage(
      W, H, x_gt[i], Q, gains[i], offsets[i], 1.0, 100u + static_cast<uint32_t>(i));

    // Perturbations balanced around (0, 0) in exact canonical frame Q (fixes track gauge at x_gt)
    const Eigen::Vector2d x_init = x_gt[i] + Q * init_offsets[i];
    feats_provider.feats_per_view[static_cast<IndexT>(i)] = {
      PointFeature(static_cast<float>(x_init.x()), static_cast<float>(x_init.y()))
    };
    feats_provider.orientations_per_view[static_cast<IndexT>(i)] = {
      static_cast<float>(thetas[i])
    };
    feats_provider.scales_per_view[static_cast<IndexT>(i)] = {
      static_cast<float>(scales[i])
    };
    map_tracks[0][static_cast<uint32_t>(i)] = 0u;
  }

  MultiViewLSMParams params;
  MultiViewLSMRefiner refiner(params);
  MultiViewLSMStats stats;
  EXPECT_TRUE(refiner.RefineFromImages(images, map_tracks, feats_provider, &stats));

  EXPECT_EQ(static_cast<size_t>(M), stats.observations_refined);
  EXPECT_EQ(size_t(0), stats.rejected_zncc);
  EXPECT_EQ(size_t(0), stats.rejected_shift);
  EXPECT_EQ(size_t(0), stats.rejected_affine_det);

  double sum_err = 0.0;
  double max_err = 0.0;
  for (int i = 0; i < M; ++i)
  {
    const PointFeature& pt = feats_provider.feats_per_view.at(static_cast<IndexT>(i))[0];
    const double err = (Eigen::Vector2d(pt.x(), pt.y()) - x_gt[i]).norm();
    sum_err += err;
    max_err = std::max(max_err, err);
  }
  const double mean_err = sum_err / static_cast<double>(M);
  EXPECT_TRUE(mean_err < 0.05);
  EXPECT_TRUE(max_err < 0.05);
}

/**
 * @brief Verify that Huber IRLS weighting + consensus aggregation resist 25% quadrant occlusions:
 *        unoccluded views remain < 0.05 px and occluded views converge within < 0.15 px.
 */
TEST(MultiViewLSMRefiner, PartialOcclusionRobustness)
{
  const int W = 128;
  const int H = 128;
  const int M = 8;

  const Eigen::Vector2d init_offsets[M] = {
    { 0.90, -0.70},
    {-0.90,  0.70},
    { 0.60,  0.80},
    {-0.60, -0.80},
    { 1.00,  0.20},
    {-1.00, -0.20},
    {-0.40,  0.60},
    { 0.40, -0.60}
  };

  Hash_Map<IndexT, Image<unsigned char>> images;
  Features_Provider feats_provider;
  STLMAPTracks map_tracks;
  std::vector<Eigen::Vector2d> x_gt(M);

  for (int i = 0; i < M; ++i)
  {
    x_gt[i] = Eigen::Vector2d(64.15, 64.35);
    Image<unsigned char> img = RenderWarpedPatchImage(
      W, H, x_gt[i], Eigen::Matrix2d::Identity(), 1.0, 0.0, 0.5, 200u + static_cast<uint32_t>(i));

    // Apply 25% quadrant occlusion on views 6 and 7 (neutral gray value so unoccluded 75% passes ZNCC)
    if (i >= 6)
    {
      for (int y = 65; y < H; ++y)
      {
        for (int x = 65; x < W; ++x)
        {
          img(y, x) = 128;
        }
      }
    }

    images[static_cast<IndexT>(i)] = std::move(img);
    const Eigen::Vector2d x_init = x_gt[i] + init_offsets[i];
    feats_provider.feats_per_view[static_cast<IndexT>(i)] = {
      PointFeature(static_cast<float>(x_init.x()), static_cast<float>(x_init.y()))
    };
    map_tracks[0][static_cast<uint32_t>(i)] = 0u;
  }

  MultiViewLSMRefiner refiner;
  MultiViewLSMStats stats;
  EXPECT_TRUE(refiner.RefineFromImages(images, map_tracks, feats_provider, &stats));

  std::vector<double> occ_errs(M, 0.0);
  for (int i = 0; i < M; ++i)
  {
    const PointFeature& pt = feats_provider.feats_per_view.at(static_cast<IndexT>(i))[0];
    occ_errs[i] = (Eigen::Vector2d(pt.x(), pt.y()) - x_gt[i]).norm();
  }
  for (int i = 0; i < M; ++i)
  {
    if (i < 6)
    {
      EXPECT_TRUE(occ_errs[i] < 0.05);
    }
    else
    {
      EXPECT_TRUE(occ_errs[i] < 0.15);
    }
  }
}

/**
 * @brief Verify that every atomic validation gate (border, max shift, ZNCC, affine determinant,
 *        low contrast) rejects anomalous observations and strictly preserves their initial coordinates.
 */
TEST(MultiViewLSMRefiner, AtomicRejectionGates)
{
  const int W = 128;
  const int H = 128;
  const Eigen::Vector2d center(64.0, 64.0);

  // 1. Border + Excess Shift + Decorrelated (ZNCC) + Extreme Affine Scale gates
  {
    Hash_Map<IndexT, Image<unsigned char>> images;
    Features_Provider feats_provider;
    STLMAPTracks map_tracks;

    // Views 0..2: valid clean observations with small opposite shifts
    const Eigen::Vector2d good_offsets[3] = {{0.6, 0.0}, {-0.6, 0.0}, {0.0, 0.4}};
    for (int i = 0; i < 3; ++i)
    {
      images[static_cast<IndexT>(i)] = RenderWarpedPatchImage(
        W, H, center, Eigen::Matrix2d::Identity(), 1.0, 0.0, 0.0, 300u + static_cast<uint32_t>(i));
      const Eigen::Vector2d p = center + good_offsets[i];
      feats_provider.feats_per_view[static_cast<IndexT>(i)] = {
        PointFeature(static_cast<float>(p.x()), static_cast<float>(p.y()))
      };
      map_tracks[0][static_cast<uint32_t>(i)] = 0u;
    }

    // View 3: border observation (within guard_radius=15 of image edge)
    images[3] = RenderWarpedPatchImage(W, H, center, Eigen::Matrix2d::Identity(), 1.0, 0.0, 0.0, 303u);
    const PointFeature border_init(5.0f, 5.0f);
    feats_provider.feats_per_view[3] = {border_init};
    map_tracks[0][3u] = 0u;

    // View 4: pure uncorrelated noise (ZNCC rejection)
    Image<unsigned char> noise_img(W, H);
    std::mt19937 rng(42u);
    std::uniform_int_distribution<int> udist(20, 235);
    for (int y = 0; y < H; ++y)
      for (int x = 0; x < W; ++x)
        noise_img(y, x) = static_cast<unsigned char>(udist(rng));
    images[4] = std::move(noise_img);
    const PointFeature noise_init(64.0f, 64.0f);
    feats_provider.feats_per_view[4] = {noise_init};
    map_tracks[0][4u] = 0u;

    // View 5: excessive initial shift (3.2 px > max_shift_px = 2.5 px)
    images[5] = RenderWarpedPatchImage(W, H, center, Eigen::Matrix2d::Identity(), 1.0, 0.0, 0.0, 305u);
    const PointFeature shift_init(67.2f, 64.0f);
    feats_provider.feats_per_view[5] = {shift_init};
    map_tracks[0][5u] = 0u;

    // View 6: extreme scale distortion (0.25x -> det = 0.0625 < 0.5)
    images[6] = RenderWarpedPatchImage(
      W, H, center, 0.25 * Eigen::Matrix2d::Identity(), 1.0, 0.0, 0.0, 306u);
    const PointFeature det_init(64.0f, 64.0f);
    feats_provider.feats_per_view[6] = {det_init};
    map_tracks[0][6u] = 0u;

    MultiViewLSMRefiner refiner;
    MultiViewLSMStats stats;
    EXPECT_TRUE(refiner.RefineFromImages(images, map_tracks, feats_provider, &stats));

    EXPECT_EQ(size_t(3), stats.observations_refined);
    EXPECT_EQ(size_t(1), stats.rejected_border);
    EXPECT_TRUE(stats.rejected_zncc >= size_t(1));
    EXPECT_TRUE((stats.rejected_shift + stats.rejected_zncc + stats.rejected_affine_det) >= size_t(3));

    // Rejected views must keep their exact initial coordinates
    EXPECT_EQ(border_init.x(), feats_provider.feats_per_view[3][0].x());
    EXPECT_EQ(border_init.y(), feats_provider.feats_per_view[3][0].y());
    EXPECT_EQ(noise_init.x(), feats_provider.feats_per_view[4][0].x());
    EXPECT_EQ(noise_init.y(), feats_provider.feats_per_view[4][0].y());
    EXPECT_EQ(shift_init.x(), feats_provider.feats_per_view[5][0].x());
    EXPECT_EQ(shift_init.y(), feats_provider.feats_per_view[5][0].y());
    EXPECT_EQ(det_init.x(), feats_provider.feats_per_view[6][0].x());
    EXPECT_EQ(det_init.y(), feats_provider.feats_per_view[6][0].y());
  }

  // 2. Low-contrast flat image rejection
  {
    Hash_Map<IndexT, Image<unsigned char>> images;
    Features_Provider feats_provider;
    STLMAPTracks map_tracks;
    for (int i = 0; i < 3; ++i)
    {
      images[static_cast<IndexT>(i)] = Image<unsigned char>(W, H, true, 120);
      feats_provider.feats_per_view[static_cast<IndexT>(i)] = {
        PointFeature(64.25f, 63.75f)
      };
      map_tracks[0][static_cast<uint32_t>(i)] = 0u;
    }

    MultiViewLSMRefiner refiner;
    MultiViewLSMStats stats;
    EXPECT_TRUE(refiner.RefineFromImages(images, map_tracks, feats_provider, &stats));
    EXPECT_EQ(size_t(0), stats.observations_refined);
    EXPECT_EQ(size_t(3), stats.rejected_low_contrast);
    EXPECT_EQ(64.25f, feats_provider.feats_per_view[0][0].x());
    EXPECT_EQ(63.75f, feats_provider.feats_per_view[0][0].y());
  }
}

/**
 * @brief Verify aperture-problem handling on pure 1D edges: 2-DoF fallback activates,
 *        perpendicular alignment converges (< 0.05 px), and tangential drift stays < 0.05 px.
 */
TEST(MultiViewLSMRefiner, ApertureProblem1DEdge)
{
  const int W = 128;
  const int H = 128;
  const int M = 5;

  const double thetas[M] = {0.0, 0.4, 1.1, 2.2, 3.8};
  const double perp_offsets[M] = {0.9, -0.9, 0.6, -0.6, 0.0};

  Hash_Map<IndexT, Image<unsigned char>> images;
  Features_Provider feats_provider;
  STLMAPTracks map_tracks;
  std::vector<Eigen::Vector2d> x_gt(M);
  std::vector<Eigen::Vector2d> normals(M);

  for (int i = 0; i < M; ++i)
  {
    x_gt[i] = Eigen::Vector2d(64.0, 64.0);
    normals[i] = Eigen::Vector2d(std::cos(thetas[i]), std::sin(thetas[i]));
    images[static_cast<IndexT>(i)] = Render1DEdgeImage(W, H, x_gt[i], thetas[i], 1.0, 0.0);

    const Eigen::Vector2d x_init = x_gt[i] + perp_offsets[i] * normals[i];
    feats_provider.feats_per_view[static_cast<IndexT>(i)] = {
      PointFeature(static_cast<float>(x_init.x()), static_cast<float>(x_init.y()))
    };
    feats_provider.orientations_per_view[static_cast<IndexT>(i)] = {
      static_cast<float>(thetas[i])
    };
    map_tracks[0][static_cast<uint32_t>(i)] = 0u;
  }

  MultiViewLSMRefiner refiner;
  MultiViewLSMStats stats;
  EXPECT_TRUE(refiner.RefineFromImages(images, map_tracks, feats_provider, &stats));

  EXPECT_TRUE(stats.fallback_2dof_count > size_t(0));
  EXPECT_EQ(static_cast<size_t>(M), stats.observations_refined);

  for (int i = 0; i < M; ++i)
  {
    const PointFeature& pt = feats_provider.feats_per_view.at(static_cast<IndexT>(i))[0];
    const Eigen::Vector2d delta(pt.x() - x_gt[i].x(), pt.y() - x_gt[i].y());
    const double perp_err = std::abs(delta.dot(normals[i]));
    const Eigen::Vector2d tangent(-normals[i].y(), normals[i].x());
    const double tang_drift = std::abs(delta.dot(tangent));
    EXPECT_TRUE(perp_err < 0.05);
    EXPECT_TRUE(tang_drift < 0.05);
  }
}

/**
 * @brief End-to-end Phase 1 disk PNG loading + Phase 2 refinement + `.feat` save/load precision check.
 */
TEST(MultiViewLSMRefiner, DiskRoundTripAndFeatSave)
{
  const std::string tmp_dir = "test_mvlsm_disk_tmp";
  if (stlplus::folder_exists(tmp_dir))
    stlplus::folder_delete(tmp_dir, true);
  EXPECT_TRUE(stlplus::folder_create(tmp_dir));

  const int W = 128;
  const int H = 128;
  const int M = 4;
  const double thetas[M] = {0.2, 1.0, 2.3, 4.1};
  const double scales[M] = {1.8, 2.2, 2.0, 2.4};
  const double s_ref = 2.1; // median of {1.8, 2.0, 2.2, 2.4}
  const Eigen::Vector2d offsets[M] = {
    { 0.85, -0.45},
    {-0.85,  0.45},
    { 0.55,  0.75},
    {-0.55, -0.75}
  };

  SfM_Data sfm_data;
  sfm_data.s_root_path = tmp_dir;
  Features_Provider feats_provider;
  STLMAPTracks map_tracks;
  std::vector<Eigen::Vector2d> x_gt(M);

  for (int i = 0; i < M; ++i)
  {
    x_gt[i] = Eigen::Vector2d(64.3125, 63.6875);
    const double alpha = scales[i] / s_ref;
    Eigen::Matrix2d R;
    R << std::cos(thetas[i]), -std::sin(thetas[i]),
         std::sin(thetas[i]),  std::cos(thetas[i]);

    const Image<unsigned char> img = RenderWarpedPatchImage(
      W, H, x_gt[i], alpha * R, 1.05, -5.0, 0.5, 400u + static_cast<uint32_t>(i));

    const std::string img_name = "view_" + std::to_string(i) + ".png";
    const std::string img_path = stlplus::create_filespec(tmp_dir, img_name);
    EXPECT_TRUE(WriteImage(img_path.c_str(), img) != 0);

    sfm_data.views[static_cast<IndexT>(i)] =
      std::make_shared<View>(img_name, i, 0, i, W, H);

    const Eigen::Vector2d x_init = x_gt[i] + alpha * R * offsets[i];
    feats_provider.feats_per_view[static_cast<IndexT>(i)] = {
      PointFeature(static_cast<float>(x_init.x()), static_cast<float>(x_init.y()))
    };
    feats_provider.orientations_per_view[static_cast<IndexT>(i)] = {
      static_cast<float>(thetas[i])
    };
    feats_provider.scales_per_view[static_cast<IndexT>(i)] = {
      static_cast<float>(scales[i])
    };
    map_tracks[0][static_cast<uint32_t>(i)] = 0u;
  }

  MultiViewLSMRefiner refiner;
  MultiViewLSMStats stats;
  EXPECT_TRUE(refiner.Refine(sfm_data, map_tracks, feats_provider, &stats));
  EXPECT_EQ(static_cast<size_t>(M), stats.observations_refined);

  for (int i = 0; i < M; ++i)
  {
    const PointFeature& pt = feats_provider.feats_per_view.at(static_cast<IndexT>(i))[0];
    const double err = (Eigen::Vector2d(pt.x(), pt.y()) - x_gt[i]).norm();
    EXPECT_TRUE(err < 0.05);
  }

  // Test SIFT_Regions SaveFeatures / LoadFeatures round-trip precision and metadata preservation
  // at 50-Mpx-scale pixel coordinates (~8192 x 6144)
  SIFT_Regions regions;
  const float test_x = 8192.12345f;
  const float test_y = 6144.98765f;
  const float test_scale = 2.3456f;
  const float test_orient = 1.2345f;
  regions.Features().emplace_back(0.0f, 0.0f, test_scale, test_orient);
  regions.SetRegionPosition(0, test_x, test_y);

  const std::string feat_path = stlplus::create_filespec(tmp_dir, "roundtrip.feat");
  EXPECT_TRUE(regions.SaveFeatures(feat_path));

  SIFT_Regions loaded;
  EXPECT_TRUE(loaded.LoadFeatures(feat_path));
  EXPECT_EQ(size_t(1), loaded.RegionCount());
  EXPECT_NEAR(test_x, loaded.Features()[0].x(), 1e-3);
  EXPECT_NEAR(test_y, loaded.Features()[0].y(), 1e-3);
  EXPECT_NEAR(test_scale, loaded.GetRegionsScales()[0], 1e-4);
  EXPECT_NEAR(test_orient, loaded.GetRegionsOrientations()[0], 1e-4);

  stlplus::folder_delete(tmp_dir, true);
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */
