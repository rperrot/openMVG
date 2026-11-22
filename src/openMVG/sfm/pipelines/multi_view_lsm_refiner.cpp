// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/**
 * @file multi_view_lsm_refiner.cpp
 * @brief Implementation of Multi-View Least-Squares Matching (MV-LSM) track refinement.
 *
 * References:
 *  - [Gruen 1985] A. Gruen, "Adaptive least squares correlation: a powerful image matching technique",
 *    South African Journal of Photogrammetry, Remote Sensing and Cartography, 1985.
 *  - [Baltsavias 1991] E. Baltsavias, "Multiphoto geometrically constrained matching", ETH Zurich, 1991.
 *  - [Baker & Matthews 2004] S. Baker and I. Matthews, "Lucas-Kanade 20 Years On: A Unifying Framework",
 *    IJCV 56(3), 2004 (Inverse Compositional Gauss-Newton formulation).
 *  - [Keys 1981] R. Keys, "Cubic convolution interpolation for digital image processing",
 *    IEEE Trans. ASSP 29(6), 1981 (Eq. 4, $a = -0.5$).
 *  - [Huber 1964] P. J. Huber, "Robust Estimation of a Location Parameter", Annals of Math. Stat., 1964.
 *  - [Dusmanu 2020] M. Dusmanu et al., "Multi-View Optimization of Local Feature Geometry", 2020.
 *  - [Lindenberger 2021] P. Lindenberger et al., "Pixel-Perfect Structure-from-Motion with Featuremetric
 *    Refinement" (PixSfM), ICCV 2021 ($O(M)$ consensus alignment & coarse-to-fine keypoint adjustment).
 */

#include "openMVG/sfm/pipelines/multi_view_lsm_refiner.hpp"

#include "openMVG/image/image_io.hpp"
#include "openMVG/system/logger.hpp"
#include "openMVG/system/timer.hpp"

#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

#ifdef OPENMVG_USE_OPENMP
#include <omp.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
#include <map>
#include <numeric>
#include <sstream>
#include <utility>
#include <vector>

namespace openMVG {
namespace sfm {

namespace {

/**
 * @brief Metadata and Phase 1 extraction status for a single 2D observation $(view\_id, feat\_id)$.
 *
 * Each guard patch of size $(2G+1) \times (2G+1)$ is centered on integer pixel
 * $(c_x, c_y) = (\operatorname{round}(x_0), \operatorname{round}(y_0))$, storing the initial
 * sub-pixel offset $\mathbf{s}_0 = (x_0 - c_x, y_0 - c_y)^T \in [-0.5, 0.5]^2$ so that
 * any warped coordinate $\mathbf{x} = \mathbf{x}_0 + \mathbf{t} + Q\mathbf{u}$ maps to
 * guard-buffer coordinates via:
 * $$\mathbf{g}(\mathbf{u}) = (G, G)^T + \mathbf{s}_0 + \mathbf{t} + Q\mathbf{u}.$$
 */
struct ObsRecord
{
  uint32_t view_id = 0;
  uint32_t feat_id = 0;
  float x0 = 0.0f;       ///< Original x coordinate (px)
  float y0 = 0.0f;       ///< Original y coordinate (px)
  float sub_x = 0.0f;    ///< Sub-pixel offset $x_0 - \operatorname{round}(x_0) \in [-0.5, 0.5]$
  float sub_y = 0.0f;    ///< Sub-pixel offset $y_0 - \operatorname{round}(y_0) \in [-0.5, 0.5]$
  float theta = 0.0f;    ///< Dominant orientation $\theta_i$ (rad)
  float scale = 0.0f;    ///< Detection scale $s_i$ (px), 0 if unavailable
  features::PointFeature* feat_ptr = nullptr; ///< Direct pointer into `Features_Provider` (lock-free Phase 2 write)
  bool valid = false;    ///< True iff the $(2G+1)\times(2G+1)$ guard patch lies inside image bounds
};

/**
 * @brief Contiguous slice `[start_obs, start_obs + count)` inside `obs_records` representing one track.
 */
struct TrackSlice
{
  size_t start_obs = 0;
  size_t count = 0;
  TrackSlice(size_t s, size_t c) : start_obs(s), count(c) {}
};

/**
 * @brief Reference used during Phase 1 per-view extraction to scatter guard patches into `guard_storage`.
 */
struct ViewObsRef
{
  uint32_t feat_id = 0;
  size_t global_obs_idx = 0;
  ViewObsRef(uint32_t f, size_t g) : feat_id(f), global_obs_idx(g) {}
};

/**
 * @brief Optimization state for one observation during Phase 2.
 *
 * The full warp from canonical template coordinates $\mathbf{u} \in [-R, R]^2$ to image coordinates is:
 * $$W_i(\mathbf{u}; \mathbf{t}_i, M_i) = \mathbf{x}_i^{(0)} + \mathbf{t}_i + L_i^{(0)} M_i \mathbf{u},$$
 * where $L_i^{(0)} = \alpha_i^{(0)} R(\theta_i)$ is fixed per track initialization and
 * $(\mathbf{t}_i, M_i = I_2 + A_i)$ are iteratively refined.
 */
struct ObsState
{
  Eigen::Vector2f t = Eigen::Vector2f::Zero();         ///< Image-space shift $\mathbf{t}_i = (\Delta x_i, \Delta y_i)^T$ (px)
  Eigen::Matrix2f M = Eigen::Matrix2f::Identity();     ///< Residual affine warp $M_i = I_2 + A_i$
  Eigen::Matrix2f L0 = Eigen::Matrix2f::Identity();    ///< Initial similarity $L_i^{(0)} = \alpha_i^{(0)} R(\theta_i)$
  bool level_active = false;  ///< True if initial sampled patch at this pyramid level is valid & textured
  bool used_2dof = false;     ///< True if 2-DoF aperture fallback triggered on this observation
  bool low_contrast = false;  ///< True if raw patch std fell below `min_patch_std`
  float last_zncc = -1.0f;    ///< Latest ZNCC against the consensus template
};

/**
 * @brief Pre-allocated per-OpenMP-worker scratch buffers so Phase 2 performs zero dynamic allocations
 *        inside track and Gauss-Newton loops.
 */
struct ThreadScratch
{
  std::vector<float> blurred_guards;   ///< Coarse Gaussian-smoothed guard patches ($M_{\max} \times S_G^2$)
  std::vector<float> fine_guards;      ///< Raw float-converted guard patches ($M_{\max} \times S_G^2$)
  std::vector<float> blur_tmp;         ///< Intermediate buffer for separable 1D Gaussian blur ($S_G^2$)
  std::vector<float> templ;            ///< Current standardized consensus template $T(\mathbf{u})$ ($N_{\text{px}}$)
  std::vector<float> templ_gx;         ///< Template spatial gradient $\partial T / \partial u_x$ ($N_{\text{px}}$)
  std::vector<float> templ_gy;         ///< Template spatial gradient $\partial T / \partial u_y$ ($N_{\text{px}}$)
  std::vector<float> jacobian;         ///< Precomputed IC-LK Jacobian $J_T(\mathbf{u}) \in \mathbb{R}^{6 \times N_{\text{px}}}$
  std::vector<float> warped_patches;   ///< Initial normalized patches per observation ($M_{\max} \times N_{\text{px}}$)
  std::vector<float> raw_patch;        ///< Workspace for newly sampled raw patch ($N_{\text{px}}$)
  std::vector<float> norm_patch;       ///< Workspace for standardized patch $\hat{I}_i$ ($N_{\text{px}}$)
  std::vector<float> residuals;        ///< Photometric residuals $r_k = \hat{I}_i(\mathbf{u}_k) - T(\mathbf{u}_k)$
  std::vector<float> weights;          ///< Combined spatial $\times$ Huber IRLS weights $w_k$
  std::vector<float> templ_accum;      ///< Weighted intensity numerator for consensus update ($N_{\text{px}}$)
  std::vector<float> templ_weight_sum; ///< Sum of pixel weights for consensus update ($N_{\text{px}}$)
  std::vector<float> scales_tmp;       ///< Scratch list for computing median track scale $s_{\text{ref}}$
  std::vector<std::pair<float, float>> median_x; ///< `(d_tpl.x, weight)` pairs for canonical gauge centering
  std::vector<std::pair<float, float>> median_y; ///< `(d_tpl.y, weight)` pairs for canonical gauge centering
  std::vector<size_t> active_indices;  ///< Scratch list of active observation indices for medoid selection
  std::vector<size_t> inliers;         ///< Scratch list of inlier indices during consensus update
  std::vector<ObsState> states;        ///< Per-observation warp state ($M_{\max}$)
  std::vector<float> local_shifts;     ///< Accepted shift magnitudes accumulated by this worker thread
  size_t tracks_processed = 0;
  size_t rejected_border = 0;
  size_t rejected_low_contrast = 0;
  size_t rejected_shift = 0;
  size_t rejected_affine_det = 0;
  size_t rejected_zncc = 0;
  size_t fallback_2dof_count = 0;

  /**
   * @brief Allocate all worker scratch buffers once before entering the parallel Phase 2 loop.
   */
  void InitForTracks(size_t max_track_len, int guard_size, int patch_size)
  {
    const size_t guard_area = static_cast<size_t>(guard_size) * guard_size;
    const size_t patch_area = static_cast<size_t>(patch_size) * patch_size;
    blurred_guards.resize(max_track_len * guard_area);
    fine_guards.resize(max_track_len * guard_area);
    blur_tmp.resize(guard_area);
    templ.resize(patch_area);
    templ_gx.resize(patch_area);
    templ_gy.resize(patch_area);
    jacobian.resize(6 * patch_area);
    warped_patches.resize(max_track_len * patch_area);
    raw_patch.resize(patch_area);
    norm_patch.resize(patch_area);
    residuals.resize(patch_area);
    weights.resize(patch_area);
    templ_accum.resize(patch_area);
    templ_weight_sum.resize(patch_area);
    scales_tmp.reserve(max_track_len);
    median_x.reserve(max_track_len);
    median_y.reserve(max_track_len);
    active_indices.reserve(max_track_len);
    inliers.reserve(max_track_len);
    states.resize(max_track_len);
  }
};

/**
 * @brief Validate configuration sanity prior to memory allocation or optimization.
 *
 * Ensures $G \ge R + 2$ so central/bicubic interpolation stencils remain inside the guard buffer,
 * and ensures all thresholds (`huber_delta`, `det_min/max`, `max_affine_cond`) are well-posed.
 */
bool ValidateParams(const MultiViewLSMParams& params)
{
  if (params.patch_radius < 2)
  {
    OPENMVG_LOG_ERROR << "MultiViewLSMParams: patch_radius must be >= 2";
    return false;
  }
  if (params.guard_radius < params.patch_radius + 2)
  {
    OPENMVG_LOG_ERROR << "MultiViewLSMParams: guard_radius must be >= patch_radius + 2";
    return false;
  }
  if (params.max_iterations < 1 || params.outer_iterations < 1)
  {
    OPENMVG_LOG_ERROR << "MultiViewLSMParams: max_iterations and outer_iterations must be >= 1";
    return false;
  }
  if (params.min_track_length < 2)
  {
    OPENMVG_LOG_ERROR << "MultiViewLSMParams: min_track_length must be >= 2";
    return false;
  }
  if (params.max_shift_px < 0.0f || params.huber_delta <= 0.0f || params.det_min <= 0.0f ||
      params.det_max < params.det_min || params.affine_reg < 0.0f || params.max_affine_cond <= 1.0f)
  {
    OPENMVG_LOG_ERROR << "MultiViewLSMParams: invalid numerical threshold parameters";
    return false;
  }
  return true;
}

/**
 * @brief Compute normalized centered 2D Gaussian spatial weights over $[-R, R]^2$.
 *
 * Computes:
 * $$w_G(\mathbf{u}) = \frac{1}{Z}\exp\!\left(-\frac{u_x^2 + u_y^2}{2\sigma_s^2}\right), \qquad
 * \sum_{\mathbf{u} \in [-R, R]^2} w_G(\mathbf{u}) = 1.$$
 * Unit-sum normalization makes the data Hessian $H_{\text{data}}$ invariant to patch size $R$,
 * keeping the affine Tikhonov parameter $\lambda_{\text{reg}}$ well-calibrated across radii.
 */
std::vector<float> ComputeSpatialWeights(const int patch_radius, const float spatial_sigma_param)
{
  const int patch_size = 2 * patch_radius + 1;
  const float sigma =
    (spatial_sigma_param > 0.0f) ? spatial_sigma_param : 0.5f * static_cast<float>(patch_radius);
  const float inv_two_sigma2 = 1.0f / (2.0f * sigma * sigma);
  std::vector<float> weights(static_cast<size_t>(patch_size) * patch_size);

  double sum = 0.0;
  for (int uy = -patch_radius; uy <= patch_radius; ++uy)
  {
    for (int ux = -patch_radius; ux <= patch_radius; ++ux)
    {
      const float r2 = static_cast<float>(ux * ux + uy * uy);
      const float w = std::exp(-r2 * inv_two_sigma2);
      weights[static_cast<size_t>(uy + patch_radius) * patch_size + (ux + patch_radius)] = w;
      sum += static_cast<double>(w);
    }
  }
  const float inv_sum = (sum > 0.0) ? static_cast<float>(1.0 / sum) : 1.0f;
  for (float& w : weights)
  {
    w *= inv_sum;
  }
  return weights;
}

/**
 * @brief Build a normalized 1D Gaussian convolution kernel truncated at $\pm \lceil 3\sigma \rceil$.
 */
void BuildGaussianKernel1D(const float sigma, std::vector<float>& kernel, int& half_width)
{
  half_width = std::max(1, static_cast<int>(std::ceil(3.0f * sigma)));
  kernel.resize(static_cast<size_t>(2 * half_width + 1));
  const float inv_two_sigma2 = 1.0f / (2.0f * sigma * sigma);
  double sum = 0.0;
  for (int k = -half_width; k <= half_width; ++k)
  {
    const float val = std::exp(-static_cast<float>(k * k) * inv_two_sigma2);
    kernel[static_cast<size_t>(k + half_width)] = val;
    sum += static_cast<double>(val);
  }
  const float inv_sum = static_cast<float>(1.0 / sum);
  for (float& v : kernel)
  {
    v *= inv_sum;
  }
}

/**
 * @brief Convert a raw `uint8_t` guard buffer directly into `float` for the fine (1:1) pyramid level.
 */
void CopyGuardToFloat(const uint8_t* src, const int guard_area, float* dst)
{
  for (int i = 0; i < guard_area; ++i)
  {
    dst[i] = static_cast<float>(src[i]);
  }
}

/**
 * @brief Apply separable 2D Gaussian smoothing $G_\sigma * I_{\text{guard}}$ for the coarse pyramid level.
 *
 * Uses clamp-to-edge padding on the outer guard perimeter (`[0, guard_size - 1]`) and writes via
 * caller-owned `tmp_buf` so zero allocations occur inside Phase 2.
 */
void BlurGuardPatch(
  const uint8_t* src,
  const int guard_size,
  const std::vector<float>& kernel,
  const int half_width,
  float* tmp_buf,
  float* dst)
{
  // Pass 1: 1D horizontal convolution (uint8_t -> float)
  for (int y = 0; y < guard_size; ++y)
  {
    const uint8_t* row_in = src + y * guard_size;
    float* row_tmp = tmp_buf + y * guard_size;
    for (int x = 0; x < guard_size; ++x)
    {
      float acc = 0.0f;
      for (int k = -half_width; k <= half_width; ++k)
      {
        const int sx = std::min(guard_size - 1, std::max(0, x + k));
        acc += kernel[static_cast<size_t>(k + half_width)] * static_cast<float>(row_in[sx]);
      }
      row_tmp[x] = acc;
    }
  }
  // Pass 2: 1D vertical convolution (float -> float)
  for (int y = 0; y < guard_size; ++y)
  {
    float* row_out = dst + y * guard_size;
    for (int x = 0; x < guard_size; ++x)
    {
      float acc = 0.0f;
      for (int k = -half_width; k <= half_width; ++k)
      {
        const int sy = std::min(guard_size - 1, std::max(0, y + k));
        acc += kernel[static_cast<size_t>(k + half_width)] * tmp_buf[sy * guard_size + x];
      }
      row_out[x] = acc;
    }
  }
}

/**
 * @brief Compute 1D Keys cubic convolution weights for fractional offset $t \in [0, 1)$ ($a = -0.5$).
 *
 * Implements Keys (1981) Eq. 4 (`SamplerCubic` convention in `openMVG/image/sample.hpp`)
 * with sharpness $a = -0.5$ (Catmull-Rom, third-order approximation precision):
 * $$w_{-1}(t) = -\tfrac{1}{2}t^3 + t^2 - \tfrac{1}{2}t, \quad
 *   w_0(t) = \tfrac{3}{2}t^3 - \tfrac{5}{2}t^2 + 1, \quad
 *   w_1(t) = -\tfrac{3}{2}t^3 + 2t^2 + \tfrac{1}{2}t, \quad
 *   w_2(t) = \tfrac{1}{2}t^3 - \tfrac{1}{2}t^2,$$
 * satisfying exact partition of unity $\sum_{k=-1}^{2} w_k(t) = 1$.
 */
inline void CubicWeightsKeys(const float t, float w[4])
{
  const float t2 = t * t;
  const float t3 = t2 * t;
  w[0] = -0.5f * t3 + 1.0f * t2 - 0.5f * t;
  w[1] = 1.5f * t3 - 2.5f * t2 + 1.0f;
  w[2] = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
  w[3] = 0.5f * t3 - 0.5f * t2;
}

/**
 * @brief Resample a $(2R+1) \times (2R+1)$ patch from a guard buffer under an affine warp.
 *
 * For each canonical template coordinate $\mathbf{u} = (u_x, u_y)^T \in \{-R,\dots,R\}^2$, evaluates:
 * $$\mathbf{g}(\mathbf{u}) = \begin{pmatrix} g_{x,0} \\ g_{y,0} \end{pmatrix} + Q \begin{pmatrix} u_x \\ u_y \end{pmatrix},$$
 * where $(g_{x,0}, g_{y,0}) = (G, G) + \mathbf{s}_0 + \mathbf{t}_i$ and $Q = L_i^{(0)} M_i$.
 *
 * @return `false` if any sample coordinate is non-finite or its interpolation stencil
 *         (`[ix, ix+1]` for bilinear, `[ix-1, ix+2]` for bicubic) exits `[0, guard_size - 1]`.
 */
bool SamplePatch(
  const float* guard,
  const int guard_size,
  const int patch_radius,
  const float gx0,
  const float gy0,
  const Eigen::Matrix2f& Q,
  const bool use_bicubic,
  float* out_patch)
{
  const float q00 = Q(0, 0);
  const float q01 = Q(0, 1);
  const float q10 = Q(1, 0);
  const float q11 = Q(1, 1);

  int idx = 0;
  for (int uy = -patch_radius; uy <= patch_radius; ++uy)
  {
    const float fuy = static_cast<float>(uy);
    const float base_x = gx0 + q01 * fuy;
    const float base_y = gy0 + q11 * fuy;
    for (int ux = -patch_radius; ux <= patch_radius; ++ux)
    {
      const float fux = static_cast<float>(ux);
      const float gx = base_x + q00 * fux;
      const float gy = base_y + q10 * fux;
      if (!std::isfinite(gx) || !std::isfinite(gy))
        return false;

      const int ix = static_cast<int>(std::floor(gx));
      const int iy = static_cast<int>(std::floor(gy));
      const float fx = gx - static_cast<float>(ix);
      const float fy = gy - static_cast<float>(iy);

      if (!use_bicubic)
      {
        if (ix < 0 || ix + 1 >= guard_size || iy < 0 || iy + 1 >= guard_size)
          return false;
        const float* r0 = guard + iy * guard_size + ix;
        const float* r1 = r0 + guard_size;
        const float v0 = r0[0] * (1.0f - fx) + r0[1] * fx;
        const float v1 = r1[0] * (1.0f - fx) + r1[1] * fx;
        out_patch[idx++] = v0 * (1.0f - fy) + v1 * fy;
      }
      else
      {
        if (ix < 1 || ix + 2 >= guard_size || iy < 1 || iy + 2 >= guard_size)
          return false;
        float wx[4];
        float wy[4];
        CubicWeightsKeys(fx, wx);
        CubicWeightsKeys(fy, wy);
        float val = 0.0f;
        for (int dy = -1; dy <= 2; ++dy)
        {
          const float* row = guard + (iy + dy) * guard_size + (ix - 1);
          const float row_val =
            wx[0] * row[0] + wx[1] * row[1] + wx[2] * row[2] + wx[3] * row[3];
          val += wy[dy + 1] * row_val;
        }
        out_patch[idx++] = val;
      }
    }
  }
  return true;
}

/**
 * @brief Compute unweighted sample standard deviation $\sigma = \sqrt{\frac{1}{N}\sum (x_k - \bar{x})^2}$
 *        in gray levels, used to detect low-contrast/flat image patches.
 */
float ComputeRawStd(const float* src, const int n)
{
  double sum = 0.0;
  double sum_sq = 0.0;
  for (int i = 0; i < n; ++i)
  {
    const double v = static_cast<double>(src[i]);
    sum += v;
    sum_sq += v * v;
  }
  const double mean = sum / static_cast<double>(n);
  const double var = std::max(0.0, sum_sq / static_cast<double>(n) - mean * mean);
  return static_cast<float>(std::sqrt(var));
}

/**
 * @brief Compute weighted mean and standard deviation:
 *        $$\mu_w = \frac{\sum_k w_k x_k}{\sum_k w_k}, \qquad
 *          \sigma_w = \sqrt{\frac{\sum_k w_k (x_k - \mu_w)^2}{\sum_k w_k}}.$$
 */
bool ComputeWeightedMeanStd(
  const float* data,
  const float* weights,
  const int n,
  float& out_mean,
  float& out_std)
{
  double w_sum = 0.0;
  double wx_sum = 0.0;
  for (int i = 0; i < n; ++i)
  {
    const double w = static_cast<double>(weights[i]);
    w_sum += w;
    wx_sum += w * static_cast<double>(data[i]);
  }
  if (w_sum <= 1e-12)
    return false;
  const double mean = wx_sum / w_sum;
  double var_acc = 0.0;
  for (int i = 0; i < n; ++i)
  {
    const double d = static_cast<double>(data[i]) - mean;
    var_acc += static_cast<double>(weights[i]) * d * d;
  }
  out_mean = static_cast<float>(mean);
  out_std = static_cast<float>(std::sqrt(std::max(0.0, var_acc / w_sum)));
  return out_std > 1e-6f;
}

/**
 * @brief Standardize a raw patch to weighted zero mean and unit variance:
 *        $$\hat{I}_k = \frac{I_k - \mu_w}{\sigma_w + \epsilon}.$$
 * @return `false` if raw gray-level std is below `min_raw_std` (flat region) or weighted variance vanishes.
 */
bool NormalizePatchWeighted(
  const float* src,
  const float* weights,
  const int n,
  const float min_raw_std,
  float* dst)
{
  if (ComputeRawStd(src, n) < min_raw_std)
    return false;
  float mean = 0.0f;
  float std_val = 0.0f;
  if (!ComputeWeightedMeanStd(src, weights, n, mean, std_val))
    return false;
  const float inv_std = 1.0f / (std_val + 1e-6f);
  for (int i = 0; i < n; ++i)
  {
    dst[i] = (src[i] - mean) * inv_std;
  }
  return true;
}

/**
 * @brief Photometrically align a sampled patch `src` to template `templ` under robust weights `weights`.
 *
 * Matches the weighted first two moments $(\mu_{\text{src}}, \sigma_{\text{src}})$ to
 * $(\mu_T, \sigma_T)$ on the same support:
 * $$\hat{I}_k = \frac{I_k - \mu_{\text{src},w}}{\sigma_{\text{src},w} + \epsilon}\,\sigma_{T,w} + \mu_{T,w}.$$
 *
 * Why weighted moment matching matters for occlusions:
 * During IRLS (`iter >= 1`), `weights` equals $w_G(\mathbf{u}) \cdot w_{\text{Huber}}(r)$, which
 * suppresses occluded pixels (e.g. bright sky silhouettes). Computing $(\mu, \sigma)$ on the
 * unoccluded support eliminates the global exposure bias that occluded pixels would otherwise inject
 * into every inlier residual. Falls back to `fallback_weights` ($w_G$) if weighted std collapses.
 */
bool NormalizePatchToTemplate(
  const float* src,
  const float* templ,
  const float* weights,
  const float* fallback_weights,
  const int n,
  const float min_raw_std,
  float* dst)
{
  if (ComputeRawStd(src, n) < min_raw_std)
    return false;

  float mu_src = 0.0f;
  float std_src = 0.0f;
  float mu_tpl = 0.0f;
  float std_tpl = 0.0f;
  bool ok = ComputeWeightedMeanStd(src, weights, n, mu_src, std_src) &&
            ComputeWeightedMeanStd(templ, weights, n, mu_tpl, std_tpl) &&
            (std_tpl >= 0.25f) && (std_src >= 0.25f * min_raw_std);

  if (!ok)
  {
    if (!ComputeWeightedMeanStd(src, fallback_weights, n, mu_src, std_src) ||
        !ComputeWeightedMeanStd(templ, fallback_weights, n, mu_tpl, std_tpl))
      return false;
  }

  const float scale = std_tpl / (std_src + 1e-6f);
  for (int i = 0; i < n; ++i)
  {
    dst[i] = (src[i] - mu_src) * scale + mu_tpl;
  }
  return true;
}

/**
 * @brief Compute unweighted Zero-Normalized Cross-Correlation (Pearson correlation) over all $N$ pixels:
 *        $$\text{ZNCC}(A, B) = \frac{\sum_k (A_k - \bar{A})(B_k - \bar{B})}
 *                                  {\sqrt{\sum_k (A_k - \bar{A})^2 \sum_k (B_k - \bar{B})^2}} \in [-1, 1].$$
 * Always unweighted so occluded/mismatched regions cannot bypass the atomic `min_zncc` gate.
 */
float ComputeZNCC(const float* a, const float* b, const int n)
{
  double sa = 0.0;
  double sb = 0.0;
  for (int i = 0; i < n; ++i)
  {
    sa += static_cast<double>(a[i]);
    sb += static_cast<double>(b[i]);
  }
  const double ma = sa / static_cast<double>(n);
  const double mb = sb / static_cast<double>(n);

  double num = 0.0;
  double va = 0.0;
  double vb = 0.0;
  for (int i = 0; i < n; ++i)
  {
    const double da = static_cast<double>(a[i]) - ma;
    const double db = static_cast<double>(b[i]) - mb;
    num += da * db;
    va += da * da;
    vb += db * db;
  }
  const double denom = std::sqrt(va * vb);
  if (denom <= 1e-12)
    return -1.0f;
  return static_cast<float>(num / denom);
}

/**
 * @brief IRLS weight $w_{\text{Huber}}(r) = \rho'_\delta(r) / r$ for the Huber robust loss (Huber 1964):
 *        $$\rho_\delta(r) = \begin{cases} \tfrac{1}{2}r^2 & |r| \le \delta \\ \delta(|r| - \tfrac{1}{2}\delta) & |r| > \delta \end{cases}
 *          \quad\Longrightarrow\quad
 *          w_{\text{Huber}}(r) = \begin{cases} 1 & |r| \le \delta \\ \delta / |r| & |r| > \delta. \end{cases}$$
 */
inline float HuberWeight(const float r, const float delta)
{
  const float abs_r = std::abs(r);
  return (abs_r <= delta) ? 1.0f : (delta / abs_r);
}

/**
 * @brief Precompute spatial gradients $\nabla T(\mathbf{u})$ and the 6-DoF IC-LK steepest-descent Jacobian.
 *
 * Uses central differences on interior template pixels and one-sided differences on the 1-px rim.
 * For the template-side incremental affine warp
 * $\Delta W(\mathbf{u}; \Delta\mathbf{p}) = \mathbf{u} + \Delta\mathbf{t} + \Delta A\,\mathbf{u}$
 * with $\Delta\mathbf{p} = (\Delta t_x, \Delta t_y, \Delta a_{11}, \Delta a_{12}, \Delta a_{21}, \Delta a_{22})^T$,
 * the first-order Taylor expansion gives (Baker & Matthews 2004):
 * $$J_T(\mathbf{u}) = \nabla T(\mathbf{u})\,\frac{\partial \Delta W}{\partial \Delta\mathbf{p}}
 *                   = \begin{bmatrix} T_x & T_y & u_x T_x & u_y T_x & u_x T_y & u_y T_y \end{bmatrix}.$$
 * Precomputed once per outer consensus update, avoiding per-iteration image gradient evaluations.
 */
void ComputeTemplateGradientsAndJacobian(
  const float* templ,
  const int patch_radius,
  float* gx_out,
  float* gy_out,
  float* jac_out)
{
  const int patch_size = 2 * patch_radius + 1;
  const int patch_area = patch_size * patch_size;

  for (int y = 0; y < patch_size; ++y)
  {
    const int y_prev = (y > 0) ? (y - 1) : 0;
    const int y_next = (y + 1 < patch_size) ? (y + 1) : (patch_size - 1);
    const float dy_scale = (y > 0 && y + 1 < patch_size) ? 0.5f : 1.0f;
    const float uy = static_cast<float>(y - patch_radius);

    for (int x = 0; x < patch_size; ++x)
    {
      const int x_prev = (x > 0) ? (x - 1) : 0;
      const int x_next = (x + 1 < patch_size) ? (x + 1) : (patch_size - 1);
      const float dx_scale = (x > 0 && x + 1 < patch_size) ? 0.5f : 1.0f;
      const float ux = static_cast<float>(x - patch_radius);

      const int k = y * patch_size + x;
      const float gx = (templ[y * patch_size + x_next] - templ[y * patch_size + x_prev]) * dx_scale;
      const float gy = (templ[y_next * patch_size + x] - templ[y_prev * patch_size + x]) * dy_scale;
      gx_out[k] = gx;
      gy_out[k] = gy;

      jac_out[0 * patch_area + k] = gx;
      jac_out[1 * patch_area + k] = gy;
      jac_out[2 * patch_area + k] = ux * gx;
      jac_out[3 * patch_area + k] = uy * gx;
      jac_out[4 * patch_area + k] = ux * gy;
      jac_out[5 * patch_area + k] = uy * gy;
    }
  }
}

/**
 * @brief Compute interior squared gradient energy $\sum \|\nabla P(\mathbf{u})\|_2^2$ (tie-breaker for $M=2$).
 */
float ComputeGradientEnergy(const float* patch, const int patch_size)
{
  double energy = 0.0;
  for (int y = 1; y + 1 < patch_size; ++y)
  {
    for (int x = 1; x + 1 < patch_size; ++x)
    {
      const float gx = 0.5f * (patch[y * patch_size + (x + 1)] - patch[y * patch_size + (x - 1)]);
      const float gy = 0.5f * (patch[(y + 1) * patch_size + x] - patch[(y - 1) * patch_size + x]);
      energy += static_cast<double>(gx * gx + gy * gy);
    }
  }
  return static_cast<float>(energy);
}

/**
 * @brief Select an unblurred/unbiased initial template $T^{(0)}$ as the track medoid.
 *
 * Instead of averaging initial unaligned patches (which blurs away high frequencies when initial
 * detections differ by $\pm 1.5\text{ px}$), picks the observation maximizing
 * $\sum_{j \neq i} \max(0, \text{ZNCC}(P_i, P_j))$. For $M = 2$ (where both ZNCC scores are equal),
 * selects the view with higher gradient energy.
 */
size_t SelectMedoidTemplate(
  const std::vector<ObsState>& states,
  const size_t obs_count,
  const float* warped_patches,
  const int patch_size,
  std::vector<size_t>& active_indices)
{
  const int patch_area = patch_size * patch_size;
  active_indices.clear();
  for (size_t i = 0; i < obs_count; ++i)
  {
    if (states[i].level_active)
      active_indices.push_back(i);
  }
  if (active_indices.empty())
    return 0;

  if (active_indices.size() == 2)
  {
    const float e0 = ComputeGradientEnergy(warped_patches + active_indices[0] * patch_area, patch_size);
    const float e1 = ComputeGradientEnergy(warped_patches + active_indices[1] * patch_area, patch_size);
    return (e0 >= e1) ? active_indices[0] : active_indices[1];
  }

  size_t best_idx = active_indices[0];
  double best_score = -1.0;
  for (size_t a = 0; a < active_indices.size(); ++a)
  {
    const size_t ia = active_indices[a];
    const float* pa = warped_patches + ia * patch_area;
    double score = 0.0;
    for (size_t b = 0; b < active_indices.size(); ++b)
    {
      if (a == b)
        continue;
      const float* pb = warped_patches + active_indices[b] * patch_area;
      const float zncc = ComputeZNCC(pa, pb, patch_area);
      if (zncc > 0.0f)
        score += static_cast<double>(zncc);
    }
    if (score > best_score)
    {
      best_score = score;
      best_idx = ia;
    }
  }
  return best_idx;
}

/**
 * @brief Align one observation `obs` to the current consensus template via IRLS IC-LK Gauss-Newton.
 *
 * Minimizes the robust, regularized Inverse-Compositional objective (Baker & Matthews 2004):
 * $$E(\Delta\mathbf{p}) = \frac{1}{2} \sum_{\mathbf{u}} w_G(\mathbf{u}) \rho_\delta\big(T(\Delta W(\mathbf{u}; \Delta\mathbf{p})) - \hat{I}_i(W_i(\mathbf{u}))\big)
 *                        + \frac{1}{2} \lambda_{\text{reg}} \| M_i (I_2 + \Delta A)^{-1} - I_2 \|_F^2.$$
 *
 * Linearizing around $\Delta\mathbf{p} = \mathbf{0}$ yields normal equations $H \Delta\mathbf{p} = \mathbf{b}$ with
 * $r_k = \hat{I}_i(W_i(\mathbf{u}_k)) - T(\mathbf{u}_k)$, $w_k = w_G(\mathbf{u}_k)\,w_{\text{Huber}}(r_k)$,
 * $H_{\text{data}} = \sum_k w_k J_T(\mathbf{u}_k)^T J_T(\mathbf{u}_k)$, and
 * $\mathbf{b}_{\text{data}} = \sum_k w_k J_T(\mathbf{u}_k)^T r_k$.
 *
 * All linear algebra uses stack-allocated fixed-size Eigen matrices (zero heap allocations).
 */
void AlignObservationICLK(
  const MultiViewLSMParams& params,
  const ObsRecord& obs,
  const float* guard,
  const std::vector<float>& spatial_weights,
  const bool allow_affine,
  ThreadScratch& scratch,
  ObsState& state)
{
  const int guard_size = 2 * params.guard_radius + 1;
  const int patch_size = 2 * params.patch_radius + 1;
  const int patch_area = patch_size * patch_size;
  const float guard_center = static_cast<float>(params.guard_radius);
  // Allow slightly larger excursion inside the loop (1.5x max_shift_px) so the final
  // atomic gate can cleanly distinguish and count genuine max_shift_px violations.
  const float max_step_bound = std::max(0.5f, 1.5f * params.max_shift_px);

  const float* templ = scratch.templ.data();
  const float* j0 = scratch.jacobian.data();
  const float* j1 = j0 + patch_area;
  const float* j2 = j1 + patch_area;
  const float* j3 = j2 + patch_area;
  const float* j4 = j3 + patch_area;
  const float* j5 = j4 + patch_area;

  for (int iter = 0; iter < params.max_iterations; ++iter)
  {
    const Eigen::Matrix2f Q = state.L0 * state.M;
    const float gx0 = guard_center + obs.sub_x + state.t.x();
    const float gy0 = guard_center + obs.sub_y + state.t.y();

    // 1. Sample warped patch I_i(W_i(u)); halt safely if the warp steps outside guard bounds
    if (!SamplePatch(
          guard,
          guard_size,
          params.patch_radius,
          gx0,
          gy0,
          Q,
          params.use_bicubic,
          scratch.raw_patch.data()))
    {
      break;
    }

    // 2. Photometric normalization: spatial weights on iter 0, then previous IRLS Huber weights
    const float* norm_weights = (iter == 0) ? spatial_weights.data() : scratch.weights.data();
    if (!NormalizePatchToTemplate(
          scratch.raw_patch.data(),
          templ,
          norm_weights,
          spatial_weights.data(),
          patch_area,
          params.min_patch_std,
          scratch.norm_patch.data()))
    {
      break;
    }

    // 3. Accumulate upper triangle of symmetric 6x6 Hessian H and gradient vector b
    Eigen::Matrix<float, 6, 6> H = Eigen::Matrix<float, 6, 6>::Zero();
    Eigen::Matrix<float, 6, 1> b = Eigen::Matrix<float, 6, 1>::Zero();

    for (int k = 0; k < patch_area; ++k)
    {
      const float r = scratch.norm_patch[k] - templ[k];
      const float w = spatial_weights[k] * HuberWeight(r, params.huber_delta);
      scratch.residuals[k] = r;
      scratch.weights[k] = w;

      const float c0 = j0[k];
      const float c1 = j1[k];
      const float c2 = j2[k];
      const float c3 = j3[k];
      const float c4 = j4[k];
      const float c5 = j5[k];
      const float wr = w * r;

      b(0) += c0 * wr;
      b(1) += c1 * wr;
      b(2) += c2 * wr;
      b(3) += c3 * wr;
      b(4) += c4 * wr;
      b(5) += c5 * wr;

      const float w0 = w * c0;
      const float w1 = w * c1;
      const float w2 = w * c2;
      const float w3 = w * c3;
      const float w4 = w * c4;
      const float w5 = w * c5;

      H(0, 0) += w0 * c0;
      H(0, 1) += w0 * c1;
      H(0, 2) += w0 * c2;
      H(0, 3) += w0 * c3;
      H(0, 4) += w0 * c4;
      H(0, 5) += w0 * c5;

      H(1, 1) += w1 * c1;
      H(1, 2) += w1 * c2;
      H(1, 3) += w1 * c3;
      H(1, 4) += w1 * c4;
      H(1, 5) += w1 * c5;

      H(2, 2) += w2 * c2;
      H(2, 3) += w2 * c3;
      H(2, 4) += w2 * c4;
      H(2, 5) += w2 * c5;

      H(3, 3) += w3 * c3;
      H(3, 4) += w3 * c4;
      H(3, 5) += w3 * c5;

      H(4, 4) += w4 * c4;
      H(4, 5) += w4 * c5;

      H(5, 5) += w5 * c5;
    }

    for (int r = 1; r < 6; ++r)
    {
      for (int c = 0; c < r; ++c)
      {
        H(r, c) = H(c, r);
      }
    }

    // 4. Aperture-problem check on the 4x4 affine block of H_data:
    //    if kappa(H_aff) = lambda_max / lambda_min > max_affine_cond (e.g. 1D edge),
    //    drop affine DoFs and solve only the well-constrained translation subspace.
    bool do_2dof = !allow_affine;
    if (!do_2dof)
    {
      const Eigen::Matrix4f H_aff_data = H.block<4, 4>(2, 2);
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix4f> eig(H_aff_data, Eigen::EigenvaluesOnly);
      if (eig.info() != Eigen::Success)
      {
        do_2dof = true;
      }
      else
      {
        const Eigen::Vector4f evals = eig.eigenvalues();
        const float lambda_min = evals(0);
        const float lambda_max = evals(3);
        if (lambda_max <= 1e-8f || lambda_min <= 1e-8f * std::max(1.0f, lambda_max) ||
            (lambda_max / lambda_min) > params.max_affine_cond)
        {
          do_2dof = true;
        }
      }
    }

    Eigen::Vector2f dt = Eigen::Vector2f::Zero();
    Eigen::Matrix2f dA = Eigen::Matrix2f::Zero();

    if (do_2dof)
    {
      // 5a. 2-DoF fallback via truncated 2x2 spectral pseudoinverse:
      //     For H_2 = sum_k lambda_k v_k v_k^T, only directions with lambda_k > max(1e-6, 0.01*lambda_max)
      //     are inverted. This eliminates tangential drift caused by uint8 quantization along 1D edges.
      state.used_2dof = true;
      const Eigen::Matrix2f H2 = H.block<2, 2>(0, 0);
      const Eigen::Vector2f b2 = b.head<2>();
      Eigen::SelfAdjointEigenSolver<Eigen::Matrix2f> eig2(H2);
      if (eig2.info() == Eigen::Success)
      {
        const Eigen::Vector2f evals = eig2.eigenvalues();
        const Eigen::Matrix2f evecs = eig2.eigenvectors();
        const float lmax = evals(1);
        if (lmax > 1e-8f)
        {
          const float thresh = std::max(1e-6f, 1e-2f * lmax);
          for (int k = 0; k < 2; ++k)
          {
            if (evals(k) > thresh)
            {
              const Eigen::Vector2f vk = evecs.col(k);
              dt += (vk.dot(b2) / evals(k)) * vk;
            }
          }
        }
      }
    }
    else
    {
      // 5b. Full 6-DoF solve with exact Tikhonov regularization toward identity on A_i = M_i - I_2:
      //     Since M_new = M_i (I_2 + dA)^{-1} \approx M_i - M_i dA, we have
      //     vec(M_new - I_2) \approx a_vec - J_reg * d_a with a_vec = vec(M_i - I_2) and
      //     J_reg = M_i \otimes I_2 (in row-major order [11, 12, 21, 22]).
      //     Minimizing 0.5 * lambda_reg * ||a_vec - J_reg * d_a||^2 adds:
      //       H_aff += lambda_reg * J_reg^T J_reg,   b_aff += lambda_reg * J_reg^T a_vec.
      if (params.affine_reg > 0.0f)
      {
        const float m00 = state.M(0, 0);
        const float m01 = state.M(0, 1);
        const float m10 = state.M(1, 0);
        const float m11 = state.M(1, 1);
        Eigen::Matrix4f J_reg = Eigen::Matrix4f::Zero();
        J_reg(0, 0) = m00;
        J_reg(0, 2) = m01;
        J_reg(1, 1) = m00;
        J_reg(1, 3) = m01;
        J_reg(2, 0) = m10;
        J_reg(2, 2) = m11;
        J_reg(3, 1) = m10;
        J_reg(3, 3) = m11;

        Eigen::Vector4f a_vec;
        a_vec << (m00 - 1.0f), m01, m10, (m11 - 1.0f);

        H.block<4, 4>(2, 2) += params.affine_reg * (J_reg.transpose() * J_reg);
        b.tail<4>() += params.affine_reg * (J_reg.transpose() * a_vec);
      }

      // Minimal diagonal Tikhonov damping for numerical Cholesky/LDLT positive-definiteness
      for (int k = 0; k < 6; ++k)
      {
        H(k, k) += 1e-6f;
      }

      const Eigen::Matrix<float, 6, 1> dp = H.ldlt().solve(b);
      if (!dp.allFinite())
        break;

      dt << dp(0), dp(1);
      dA << dp(2), dp(3), dp(4), dp(5);
    }

    if (!dt.allFinite() || !dA.allFinite())
      break;

    // 6. Inverse-compositional warp update: W_new(u) = W_i(DeltaW^{-1}(u))
    //    Since DeltaW^{-1}(v) = M_inc^{-1} (v - dt) with M_inc = I_2 + dA, composition gives:
    //      M_new = M_i * M_inc^{-1},   t_new = t_i - (L_i^{(0)} * M_new) * dt.
    const Eigen::Matrix2f M_inc = Eigen::Matrix2f::Identity() + dA;
    const float det_inc = M_inc.determinant();
    if (!std::isfinite(det_inc) || std::abs(det_inc) < 0.25f)
      break;

    const Eigen::Matrix2f M_cand = state.M * M_inc.inverse();
    const Eigen::Vector2f t_cand = state.t - (state.L0 * M_cand) * dt;
    if (!M_cand.allFinite() || !t_cand.allFinite())
      break;

    const float det_cand = M_cand.determinant();
    if (det_cand < 0.25f || det_cand > 4.0f || t_cand.norm() > max_step_bound)
      break;

    state.M = M_cand;
    state.t = t_cand;

    if (dt.norm() < params.convergence_eps && dA.norm() < 0.1f * params.convergence_eps)
      break;
  }
}

/**
 * @brief Compute a smooth, outlier-resistant weighted center via a Huber M-estimator of location.
 *
 * Solves $\min_c \sum_i w_i \rho_\delta(v_i - c)$ via 5 IRLS iterations starting from the
 * weighted mean. Unlike a discrete uninterpolated weighted median (which can jump discontinuously
 * by $\sim 0.5\text{ px}$ between adjacent values on small tracks when weights vary by $10^{-3}$),
 * this M-estimator varies smoothly with weights while remaining robust to outlier shifts and
 * downweighting partially occluded views via `w_i = ZNCC_i^4`.
 */
float ComputeRobustWeightedCenter(
  const std::vector<std::pair<float, float>>& items,
  const float huber_delta)
{
  if (items.empty())
    return 0.0f;
  double w_sum = 0.0;
  double wx_sum = 0.0;
  for (const auto& kv : items)
  {
    const double w = static_cast<double>(kv.second);
    w_sum += w;
    wx_sum += w * static_cast<double>(kv.first);
  }
  if (w_sum <= 1e-12)
    return 0.0f;
  float center = static_cast<float>(wx_sum / w_sum);
  for (int it = 0; it < 5; ++it)
  {
    double irls_w_sum = 0.0;
    double irls_wx_sum = 0.0;
    for (const auto& kv : items)
    {
      const double w =
        static_cast<double>(kv.second) * static_cast<double>(HuberWeight(kv.first - center, huber_delta));
      irls_w_sum += w;
      irls_wx_sum += w * static_cast<double>(kv.first);
    }
    if (irls_w_sum <= 1e-12)
      break;
    center = static_cast<float>(irls_wx_sum / irls_w_sum);
  }
  return center;
}

/**
 * @brief Compute the exact median of a non-empty mutable float array in $O(N)$ expected time.
 */
float ComputeMedian(std::vector<float>& vals)
{
  if (vals.empty())
    return 0.0f;
  const size_t mid = vals.size() / 2;
  std::nth_element(vals.begin(), vals.begin() + mid, vals.end());
  if ((vals.size() % 2) == 1)
    return vals[mid];
  const float upper = vals[mid];
  const float lower = *std::max_element(vals.begin(), vals.begin() + mid);
  return 0.5f * (lower + upper);
}

/**
 * @brief Fix global track gauge freedom and aggregate the updated $O(M)$ consensus template.
 *
 * Without 3D triangulation constraints, aligning $M$ views to a shared template has a 2D gauge
 * ambiguity: shifting every view by $Q_i \bar{\mathbf{d}}$ preserves all pairwise matches.
 * This routine:
 * 1. Evaluates ZNCC of each view at its converged warp and collects inliers
 *    ($\text{ZNCC}_i \ge \min(0.5, \text{min\_zncc})$, valid shift & determinant).
 * 2. Maps each inlier's displacement into the canonical template frame,
 *    $\mathbf{d}_{\text{tpl}, i} = (L_i^{(0)} M_i)^{-1} \mathbf{t}_i$, estimates the robust
 *    $\text{ZNCC}^4$-weighted center $\bar{\mathbf{d}}_{\text{tpl}}$, and recenters all warps via
 *    $\mathbf{t}_i \leftarrow \mathbf{t}_i - (L_i^{(0)} M_i)\bar{\mathbf{d}}_{\text{tpl}}$
 *    so the consensus tracks the robust consensus of initial detections without global drift.
 * 3. Immediately resamples the inliers at the recentered warps (keeping template and warps
 *    strictly synchronized) and averages them pixel-by-pixel with weights
 *    $w_{i,k} = \max(0, \text{ZNCC}_i)^2 \cdot w_{\text{Huber}}(\hat{I}_{i,k} - T_k)$ to
 *    neutralize partial occlusions before re-standardizing $T$.
 */
bool UpdateConsensusAndCenterGauge(
  const MultiViewLSMParams& params,
  const ObsRecord* track_obs,
  const size_t obs_count,
  const float* guards,
  const std::vector<float>& spatial_weights,
  ThreadScratch& scratch)
{
  const int guard_size = 2 * params.guard_radius + 1;
  const int patch_size = 2 * params.patch_radius + 1;
  const int patch_area = patch_size * patch_size;
  const float guard_center = static_cast<float>(params.guard_radius);
  const float inlier_zncc_thresh = std::min(0.5f, params.min_zncc);

  scratch.median_x.clear();
  scratch.median_y.clear();
  scratch.inliers.clear();
  std::vector<size_t>& inliers = scratch.inliers;

  // 1. Score all active observations against current template T and gather canonical shifts
  for (size_t i = 0; i < obs_count; ++i)
  {
    ObsState& st = scratch.states[i];
    if (!st.level_active)
      continue;

    const ObsRecord& obs = track_obs[i];
    const float* guard = guards + i * static_cast<size_t>(guard_size) * guard_size;
    if (!SamplePatch(
          guard,
          guard_size,
          params.patch_radius,
          guard_center + obs.sub_x + st.t.x(),
          guard_center + obs.sub_y + st.t.y(),
          st.L0 * st.M,
          params.use_bicubic,
          scratch.raw_patch.data()))
    {
      st.last_zncc = -1.0f;
      continue;
    }

    st.last_zncc = ComputeZNCC(scratch.raw_patch.data(), scratch.templ.data(), patch_area);
    const float det_m = st.M.determinant();
    if (st.last_zncc >= inlier_zncc_thresh && st.t.norm() <= params.max_shift_px &&
        det_m >= params.det_min && det_m <= params.det_max)
    {
      inliers.push_back(i);
      const Eigen::Matrix2f Q = st.L0 * st.M;
      if (std::abs(Q.determinant()) > 1e-6f)
      {
        const Eigen::Vector2f d_tpl = Q.inverse() * st.t;
        if (d_tpl.allFinite())
        {
          // Weight clean high-ZNCC views much more strongly than partially occluded ones
          const float z = std::max(0.0f, st.last_zncc);
          const float w = z * z * z * z;
          scratch.median_x.emplace_back(d_tpl.x(), w);
          scratch.median_y.emplace_back(d_tpl.y(), w);
        }
      }
    }
  }

  if (inliers.size() < 2)
    return false;

  // 2. Gauge centering: remove ZNCC-weighted Huber robust center of canonical shift across inliers
  if (!scratch.median_x.empty())
  {
    const float gauge_huber_delta = std::max(0.5f, 0.5f * params.max_shift_px);
    const Eigen::Vector2f d_bar(
      ComputeRobustWeightedCenter(scratch.median_x, gauge_huber_delta),
      ComputeRobustWeightedCenter(scratch.median_y, gauge_huber_delta));
    for (size_t i = 0; i < obs_count; ++i)
    {
      ObsState& st = scratch.states[i];
      if (!st.level_active)
        continue;
      const Eigen::Vector2f t_new = st.t - (st.L0 * st.M) * d_bar;
      if (t_new.allFinite() && t_new.norm() <= 1.5f * params.max_shift_px)
      {
        st.t = t_new;
      }
    }
  }

  // 3. Resample inliers at recentered warps and robustly aggregate the updated consensus template
  std::fill(scratch.templ_accum.begin(), scratch.templ_accum.end(), 0.0f);
  std::fill(scratch.templ_weight_sum.begin(), scratch.templ_weight_sum.end(), 0.0f);

  size_t valid_contrib = 0;
  for (const size_t i : inliers)
  {
    const ObsState& st = scratch.states[i];
    const ObsRecord& obs = track_obs[i];
    const float* guard = guards + i * static_cast<size_t>(guard_size) * guard_size;

    if (!SamplePatch(
          guard,
          guard_size,
          params.patch_radius,
          guard_center + obs.sub_x + st.t.x(),
          guard_center + obs.sub_y + st.t.y(),
          st.L0 * st.M,
          params.use_bicubic,
          scratch.raw_patch.data()))
    {
      continue;
    }

    if (!NormalizePatchToTemplate(
          scratch.raw_patch.data(),
          scratch.templ.data(),
          spatial_weights.data(),
          spatial_weights.data(),
          patch_area,
          params.min_patch_std,
          scratch.norm_patch.data()))
    {
      continue;
    }

    const float view_w = std::max(0.0f, st.last_zncc) * std::max(0.0f, st.last_zncc);
    for (int k = 0; k < patch_area; ++k)
    {
      const float r = scratch.norm_patch[k] - scratch.templ[k];
      const float w = view_w * HuberWeight(r, params.huber_delta);
      scratch.templ_accum[k] += w * scratch.norm_patch[k];
      scratch.templ_weight_sum[k] += w;
    }
    ++valid_contrib;
  }

  if (valid_contrib < 2)
    return false;

  for (int k = 0; k < patch_area; ++k)
  {
    if (scratch.templ_weight_sum[k] > 1e-8f)
    {
      scratch.templ_accum[k] /= scratch.templ_weight_sum[k];
    }
    else
    {
      scratch.templ_accum[k] = scratch.templ[k];
    }
  }

  // Standardize updated consensus template under spatial weights (mu=0, sigma=1)
  float mu = 0.0f;
  float std_val = 0.0f;
  if (!ComputeWeightedMeanStd(scratch.templ_accum.data(), spatial_weights.data(), patch_area, mu, std_val))
    return false;
  const float inv_std = 1.0f / (std_val + 1e-6f);
  for (int k = 0; k < patch_area; ++k)
  {
    scratch.templ[k] = (scratch.templ_accum[k] - mu) * inv_std;
  }
  return true;
}

/**
 * @brief Run one pyramid level (coarse or fine) over all active observations of a track.
 *
 * - Samples and normalizes initial patches at the current warps.
 * - Initializes the consensus template $T^{(0)}$ either via medoid selection (coarse or cold fine)
 *   or by robustly aggregating unblurred fine patches evaluated at the coarse warps (warm start).
 * - Executes `outer_iterations` consensus rounds: precompute $\nabla T$ & $J_T$, align every view
 *   with `AlignObservationICLK`, then update consensus & center gauge.
 */
bool RunPyramidLevel(
  const MultiViewLSMParams& params,
  const ObsRecord* track_obs,
  const size_t obs_count,
  const float* guards,
  const std::vector<float>& spatial_weights,
  const bool warm_start_from_coarse,
  ThreadScratch& scratch)
{
  const int guard_size = 2 * params.guard_radius + 1;
  const int patch_size = 2 * params.patch_radius + 1;
  const int patch_area = patch_size * patch_size;
  const float guard_center = static_cast<float>(params.guard_radius);

  // Sample and verify contrast of each observation at the entry warp of this pyramid level
  size_t active_count = 0;
  for (size_t i = 0; i < obs_count; ++i)
  {
    ObsState& st = scratch.states[i];
    st.level_active = false;
    if (!track_obs[i].valid)
      continue;

    const float* guard = guards + i * static_cast<size_t>(guard_size) * guard_size;
    float* dst_patch = scratch.warped_patches.data() + i * patch_area;
    if (!SamplePatch(
          guard,
          guard_size,
          params.patch_radius,
          guard_center + track_obs[i].sub_x + st.t.x(),
          guard_center + track_obs[i].sub_y + st.t.y(),
          st.L0 * st.M,
          params.use_bicubic,
          scratch.raw_patch.data()))
    {
      continue;
    }

    if (!NormalizePatchWeighted(
          scratch.raw_patch.data(),
          spatial_weights.data(),
          patch_area,
          params.min_patch_std,
          dst_patch))
    {
      st.low_contrast = true;
      continue;
    }
    st.low_contrast = false;
    st.level_active = true;
    ++active_count;
  }

  if (active_count < 2)
    return false;

  if (!warm_start_from_coarse)
  {
    const size_t medoid = SelectMedoidTemplate(
      scratch.states, obs_count, scratch.warped_patches.data(), patch_size, scratch.active_indices);
    const float* medoid_patch = scratch.warped_patches.data() + medoid * patch_area;
    std::copy(medoid_patch, medoid_patch + patch_area, scratch.templ.begin());
  }
  else
  {
    // Warm-start fine template directly from sharp fine patches evaluated at converged coarse warps
    const size_t medoid = SelectMedoidTemplate(
      scratch.states, obs_count, scratch.warped_patches.data(), patch_size, scratch.active_indices);
    const float* medoid_patch = scratch.warped_patches.data() + medoid * patch_area;
    std::copy(medoid_patch, medoid_patch + patch_area, scratch.templ.begin());

    std::fill(scratch.templ_accum.begin(), scratch.templ_accum.end(), 0.0f);
    std::fill(scratch.templ_weight_sum.begin(), scratch.templ_weight_sum.end(), 0.0f);
    size_t contrib = 0;
    const float min_coarse_zncc = std::min(0.5f, params.min_zncc);
    for (size_t i = 0; i < obs_count; ++i)
    {
      const ObsState& st = scratch.states[i];
      if (!st.level_active || st.last_zncc < min_coarse_zncc)
        continue;
      const float* p = scratch.warped_patches.data() + i * patch_area;
      const float vw = st.last_zncc * st.last_zncc;
      for (int k = 0; k < patch_area; ++k)
      {
        const float r = p[k] - scratch.templ[k];
        const float w = vw * HuberWeight(r, params.huber_delta);
        scratch.templ_accum[k] += w * p[k];
        scratch.templ_weight_sum[k] += w;
      }
      ++contrib;
    }
    if (contrib >= 2)
    {
      for (int k = 0; k < patch_area; ++k)
      {
        if (scratch.templ_weight_sum[k] > 1e-8f)
          scratch.templ_accum[k] /= scratch.templ_weight_sum[k];
        else
          scratch.templ_accum[k] = scratch.templ[k];
      }
      float mu = 0.0f;
      float std_val = 0.0f;
      if (ComputeWeightedMeanStd(scratch.templ_accum.data(), spatial_weights.data(), patch_area, mu, std_val))
      {
        const float inv_std = 1.0f / (std_val + 1e-6f);
        for (int k = 0; k < patch_area; ++k)
        {
          scratch.templ[k] = (scratch.templ_accum[k] - mu) * inv_std;
        }
      }
    }
  }

  // Alternating consensus optimization: IC-LK per view -> gauge recenter & template update
  for (int outer = 0; outer < params.outer_iterations; ++outer)
  {
    ComputeTemplateGradientsAndJacobian(
      scratch.templ.data(),
      params.patch_radius,
      scratch.templ_gx.data(),
      scratch.templ_gy.data(),
      scratch.jacobian.data());

    for (size_t i = 0; i < obs_count; ++i)
    {
      ObsState& st = scratch.states[i];
      if (!st.level_active)
        continue;

      const float* guard = guards + i * static_cast<size_t>(guard_size) * guard_size;
      AlignObservationICLK(params, track_obs[i], guard, spatial_weights, true, scratch, st);
    }

    UpdateConsensusAndCenterGauge(params, track_obs, obs_count, guards, spatial_weights, scratch);
  }

  return true;
}

/**
 * @brief Refine all observations of a single track and apply updates atomically per observation.
 *
 * 1. Computes median track scale $s_{\text{ref}}$ and upper scale clamp $\alpha_{\max}$ so that
 *    rotated corners remain strictly inside the guard buffer:
 *    $$\alpha_{\max} = \min\!\left(2.0,\; \max\!\left(1.0,\; \frac{G - m_{\text{interp}} - \text{max\_shift\_px}}{\sqrt{2}\,R}\right)\right).$$
 * 2. Initializes $L_i^{(0)} = \alpha_i^{(0)} R(\theta_i)$ with $\alpha_i^{(0)} = \operatorname{clamp}(s_i / s_{\text{ref}}, 0.5, \alpha_{\max})$.
 * 3. Runs coarse-to-fine IC-LK consensus alignment (`blurred_guards` -> `fine_guards`).
 * 4. Enforces 5 atomic validation gates per observation (`valid`/bounds, `min_patch_std`,
 *    $\|\mathbf{t}_i\|_2 \le \text{max\_shift\_px}$, $\det(M_i) \in [\det_{\min}, \det_{\max}]$,
 *    and $\text{ZNCC} \ge \text{min\_zncc}$); rejected observations remain strictly untouched.
 */
void RefineSingleTrack(
  const MultiViewLSMParams& params,
  const ObsRecord* track_obs,
  const size_t obs_count,
  const uint8_t* guard_storage,
  const std::vector<size_t>& track_obs_indices,
  const size_t slice_start,
  const std::vector<float>& spatial_weights,
  const std::vector<float>& coarse_kernel,
  const int coarse_half_width,
  ThreadScratch& scratch)
{
  const int guard_size = 2 * params.guard_radius + 1;
  const int guard_area = guard_size * guard_size;
  const int patch_size = 2 * params.patch_radius + 1;
  const int patch_area = patch_size * patch_size;
  const float guard_center = static_cast<float>(params.guard_radius);

  size_t valid_count = 0;
  scratch.scales_tmp.clear();
  for (size_t i = 0; i < obs_count; ++i)
  {
    if (track_obs[i].valid)
    {
      ++valid_count;
      if (track_obs[i].scale > 0.0f)
        scratch.scales_tmp.push_back(track_obs[i].scale);
    }
  }

  if (valid_count < 2)
  {
    for (size_t i = 0; i < obs_count; ++i)
    {
      if (!track_obs[i].valid)
        ++scratch.rejected_border;
    }
    return;
  }

  ++scratch.tracks_processed;

  // Derive median reference scale and safe upper bound alpha_max for guard buffer containment
  const float s_ref = scratch.scales_tmp.empty() ? 0.0f : ComputeMedian(scratch.scales_tmp);
  const float interp_margin = params.use_bicubic ? 2.0f : 1.0f;
  const float safe_reach =
    static_cast<float>(params.guard_radius) - interp_margin - params.max_shift_px;
  const float diag_radius = std::sqrt(2.0f) * static_cast<float>(params.patch_radius);
  const float alpha_max = std::min(2.0f, std::max(1.0f, safe_reach / diag_radius));

  for (size_t i = 0; i < obs_count; ++i)
  {
    const ObsRecord& obs = track_obs[i];
    ObsState& st = scratch.states[i];
    st = ObsState();

    float alpha0 = 1.0f;
    if (params.use_scale && obs.scale > 0.0f && s_ref > 0.0f)
    {
      alpha0 = std::min(alpha_max, std::max(0.5f, obs.scale / s_ref));
    }
    const float theta = params.use_orientation ? obs.theta : 0.0f;
    const float c = std::cos(theta);
    const float s = std::sin(theta);
    st.L0 << alpha0 * c, -alpha0 * s, alpha0 * s, alpha0 * c;
  }

  // Pyramid Level 0 (coarse): Gaussian-smoothed guard buffers widen the basin of attraction
  const bool use_coarse = (params.coarse_sigma > 0.0f);
  bool coarse_ok = false;

  if (use_coarse)
  {
    for (size_t i = 0; i < obs_count; ++i)
    {
      if (!track_obs[i].valid)
        continue;
      const uint8_t* src_guard =
        guard_storage + track_obs_indices[slice_start + i] * static_cast<size_t>(guard_area);
      BlurGuardPatch(
        src_guard,
        guard_size,
        coarse_kernel,
        coarse_half_width,
        scratch.blur_tmp.data(),
        scratch.blurred_guards.data() + i * guard_area);
    }

    coarse_ok = RunPyramidLevel(
      params,
      track_obs,
      obs_count,
      scratch.blurred_guards.data(),
      spatial_weights,
      false,
      scratch);

    // If coarse alignment failed (e.g. too few textured views), reset before trying fine level cold
    if (!coarse_ok)
    {
      for (size_t i = 0; i < obs_count; ++i)
      {
        scratch.states[i].t = Eigen::Vector2f::Zero();
        scratch.states[i].M = Eigen::Matrix2f::Identity();
        scratch.states[i].last_zncc = -1.0f;
      }
    }
  }

  // Pyramid Level 1 (fine): native 1:1 unsmoothed intensities for final sub-pixel precision (< 0.05 px)
  for (size_t i = 0; i < obs_count; ++i)
  {
    if (!track_obs[i].valid)
      continue;
    const uint8_t* src_guard =
      guard_storage + track_obs_indices[slice_start + i] * static_cast<size_t>(guard_area);
    CopyGuardToFloat(src_guard, guard_area, scratch.fine_guards.data() + i * guard_area);
    scratch.states[i].used_2dof = false;
  }

  const bool fine_ok = RunPyramidLevel(
    params,
    track_obs,
    obs_count,
    scratch.fine_guards.data(),
    spatial_weights,
    coarse_ok,
    scratch);

  // Atomic validation gates per observation (any failure leaves (x0, y0) unmodified)
  for (size_t i = 0; i < obs_count; ++i)
  {
    const ObsRecord& obs = track_obs[i];
    const ObsState& st = scratch.states[i];

    if (!obs.valid)
    {
      ++scratch.rejected_border;
      continue;
    }
    if (st.used_2dof)
    {
      ++scratch.fallback_2dof_count;
    }
    if (!fine_ok)
    {
      ++scratch.rejected_low_contrast;
      continue;
    }
    if (!st.t.allFinite() || !st.M.allFinite())
    {
      ++scratch.rejected_shift;
      continue;
    }

    // Gate 1: maximum total displacement norm ||t_i||_2 <= max_shift_px
    const float shift_norm = st.t.norm();
    if (shift_norm > params.max_shift_px)
    {
      ++scratch.rejected_shift;
      continue;
    }

    // Gate 2: physical residual affine determinant det(M_i) in [det_min, det_max]
    const float det_m = st.M.determinant();
    if (!std::isfinite(det_m) || det_m < params.det_min || det_m > params.det_max)
    {
      ++scratch.rejected_affine_det;
      continue;
    }

    // Gate 3: final patch remains inside guard buffer & has sufficient gray-level contrast
    const float* fine_guard = scratch.fine_guards.data() + i * guard_area;
    if (!SamplePatch(
          fine_guard,
          guard_size,
          params.patch_radius,
          guard_center + obs.sub_x + st.t.x(),
          guard_center + obs.sub_y + st.t.y(),
          st.L0 * st.M,
          params.use_bicubic,
          scratch.raw_patch.data()))
    {
      ++scratch.rejected_border;
      continue;
    }

    if (ComputeRawStd(scratch.raw_patch.data(), patch_area) < params.min_patch_std)
    {
      ++scratch.rejected_low_contrast;
      continue;
    }

    // Gate 4: unweighted final ZNCC against consensus template >= min_zncc
    const float zncc_final = ComputeZNCC(scratch.raw_patch.data(), scratch.templ.data(), patch_area);
    if (!std::isfinite(zncc_final) || zncc_final < params.min_zncc)
    {
      ++scratch.rejected_zncc;
      continue;
    }

    // Commit sub-pixel update directly into Features_Provider (lock-free: unique (view_id, feat_id))
    obs.feat_ptr->x() = obs.x0 + st.t.x();
    obs.feat_ptr->y() = obs.y0 + st.t.y();
    scratch.local_shifts.push_back(shift_norm);
  }
}

/**
 * @brief Shared two-phase MV-LSM execution driver used by both `Refine` (disk) and `RefineFromImages`.
 *
 * - Builds compact flat track slices and per-view observation indices, verifying uniqueness of every
 *   `(view_id, feat_id)` pair so Phase 2 writes into `features_provider` are race-free without locks.
 * - Phase 1 (`#pragma omp parallel for num_threads(max_io_threads)`): loads each active view once via
 *   `load_image_fn`, copies contiguous `(2G+1)^2` `uint8_t` guard patches, and discards the full image.
 * - Phase 2 (`#pragma omp parallel`): optimizes each track independently in `ThreadScratch` and merges
 *   worker statistics & applied shifts at completion.
 */
template <typename LoadImageFn>
bool RunRefinement(
  const MultiViewLSMParams& params,
  const tracks::STLMAPTracks& map_tracks,
  Features_Provider& features_provider,
  LoadImageFn&& load_image_fn,
  MultiViewLSMStats* out_stats)
{
  if (!ValidateParams(params))
    return false;

  MultiViewLSMStats stats;
  stats.tracks_total = map_tracks.size();

  if (map_tracks.empty())
  {
    if (out_stats)
      *out_stats = stats;
    return true;
  }

  system::Timer timer;

  // Build flat index structures for eligible tracks and per-view patch extraction
  std::vector<TrackSlice> track_slices;
  std::vector<ObsRecord> obs_records;
  std::vector<size_t> track_obs_indices;
  std::map<uint32_t, std::vector<ViewObsRef>> obs_per_view;

  track_slices.reserve(map_tracks.size());
  size_t max_track_len = 0;

  for (const auto& track_kv : map_tracks)
  {
    const tracks::submapTrack& track = track_kv.second;
    if (track.size() < static_cast<size_t>(params.min_track_length))
      continue;

    const size_t slice_start = obs_records.size();
    size_t slice_count = 0;

    for (const auto& obs_kv : track)
    {
      const uint32_t view_id = obs_kv.first;
      const uint32_t feat_id = obs_kv.second;

      auto feat_it = features_provider.feats_per_view.find(view_id);
      if (feat_it == features_provider.feats_per_view.end() ||
          feat_id >= feat_it->second.size())
      {
        continue;
      }

      features::PointFeature& pt = feat_it->second[feat_id];
      const float x0 = pt.x();
      const float y0 = pt.y();
      const int cx = static_cast<int>(std::lround(x0));
      const int cy = static_cast<int>(std::lround(y0));

      ObsRecord rec;
      rec.view_id = view_id;
      rec.feat_id = feat_id;
      rec.x0 = x0;
      rec.y0 = y0;
      rec.sub_x = x0 - static_cast<float>(cx);
      rec.sub_y = y0 - static_cast<float>(cy);
      rec.theta = features_provider.getFeatureOrientation(view_id, feat_id);
      rec.scale = features_provider.getFeatureScale(view_id, feat_id);
      rec.feat_ptr = &pt;
      rec.valid = false;

      const size_t global_idx = obs_records.size();
      obs_records.push_back(rec);
      track_obs_indices.push_back(global_idx);
      obs_per_view[view_id].emplace_back(feat_id, global_idx);
      ++slice_count;
    }

    if (slice_count >= static_cast<size_t>(params.min_track_length))
    {
      track_slices.emplace_back(slice_start, slice_count);
      max_track_len = std::max(max_track_len, slice_count);
    }
    else
    {
      // Rollback partial observations for tracks with missing features
      while (obs_records.size() > slice_start)
      {
        const ObsRecord& dropped = obs_records.back();
        obs_per_view[dropped.view_id].pop_back();
        obs_records.pop_back();
        track_obs_indices.pop_back();
      }
    }
  }

  stats.observations_total = obs_records.size();
  if (track_slices.empty() || obs_records.empty())
  {
    if (out_stats)
      *out_stats = stats;
    return true;
  }

  // Sort per-view references and verify uniqueness of (view_id, feat_id) for lock-free Phase 2 writes
  std::vector<uint32_t> active_view_ids;
  active_view_ids.reserve(obs_per_view.size());
  for (auto& kv : obs_per_view)
  {
    if (kv.second.empty())
      continue;
    std::sort(
      kv.second.begin(),
      kv.second.end(),
      [](const ViewObsRef& a, const ViewObsRef& b) { return a.feat_id < b.feat_id; });
    for (size_t i = 1; i < kv.second.size(); ++i)
    {
      if (kv.second[i].feat_id == kv.second[i - 1].feat_id)
      {
        OPENMVG_LOG_ERROR << "MultiViewLSMRefiner: duplicate (view_id=" << kv.first
                          << ", feat_id=" << kv.second[i].feat_id
                          << ") detected across tracks; run TracksBuilder::Filter() first.";
        return false;
      }
    }
    active_view_ids.push_back(kv.first);
  }

  // Phase 1: Single-pass per-view extraction of contiguous uint8_t guard patches
  const int guard_radius = params.guard_radius;
  const int guard_size = 2 * guard_radius + 1;
  const size_t guard_area = static_cast<size_t>(guard_size) * guard_size;
  std::vector<uint8_t> guard_storage(obs_records.size() * guard_area, uint8_t(0));

#ifdef OPENMVG_USE_OPENMP
  const int io_threads = std::max(1, std::min(omp_get_max_threads(), params.max_io_threads));
#pragma omp parallel for schedule(dynamic) num_threads(io_threads)
#endif
  for (int v_idx = 0; v_idx < static_cast<int>(active_view_ids.size()); ++v_idx)
  {
    const uint32_t view_id = active_view_ids[static_cast<size_t>(v_idx)];
    const auto& view_obs = obs_per_view.at(view_id);

    const image::Image<unsigned char>* img_ptr = nullptr;
    image::Image<unsigned char> owned_img;
    if (!load_image_fn(view_id, img_ptr, owned_img) || !img_ptr)
    {
      continue;
    }

    const int width = img_ptr->Width();
    const int height = img_ptr->Height();

    for (const ViewObsRef& ref : view_obs)
    {
      ObsRecord& rec = obs_records[ref.global_obs_idx];
      if (!std::isfinite(rec.x0) || !std::isfinite(rec.y0))
        continue;

      const int cx = static_cast<int>(std::lround(rec.x0));
      const int cy = static_cast<int>(std::lround(rec.y0));
      if (cx - guard_radius < 0 || cx + guard_radius >= width ||
          cy - guard_radius < 0 || cy + guard_radius >= height)
      {
        continue;
      }

      uint8_t* dst = guard_storage.data() + ref.global_obs_idx * guard_area;
      for (int dy = -guard_radius; dy <= guard_radius; ++dy)
      {
        const unsigned char* src_row = &(*img_ptr)(cy + dy, cx - guard_radius);
        std::memcpy(
          dst + static_cast<size_t>(dy + guard_radius) * guard_size,
          src_row,
          static_cast<size_t>(guard_size));
      }
      rec.valid = true;
    }
  }

  stats.elapsed_phase1_s = timer.elapsed();
  timer.reset();

  // Phase 2: Per-track OpenMP optimization with zero inner-loop heap allocations
  const std::vector<float> spatial_weights = ComputeSpatialWeights(params.patch_radius, params.spatial_sigma);
  std::vector<float> coarse_kernel;
  int coarse_half_width = 0;
  if (params.coarse_sigma > 0.0f)
  {
    BuildGaussianKernel1D(params.coarse_sigma, coarse_kernel, coarse_half_width);
  }

  const int patch_size = 2 * params.patch_radius + 1;
  std::vector<float> all_shifts;
  all_shifts.reserve(obs_records.size());

#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel
#endif
  {
    ThreadScratch scratch;
    scratch.InitForTracks(max_track_len, guard_size, patch_size);

#ifdef OPENMVG_USE_OPENMP
#pragma omp for schedule(dynamic, 16) nowait
#endif
    for (int t_idx = 0; t_idx < static_cast<int>(track_slices.size()); ++t_idx)
    {
      const TrackSlice& slice = track_slices[static_cast<size_t>(t_idx)];
      RefineSingleTrack(
        params,
        obs_records.data() + slice.start_obs,
        slice.count,
        guard_storage.data(),
        track_obs_indices,
        slice.start_obs,
        spatial_weights,
        coarse_kernel,
        coarse_half_width,
        scratch);
    }

#ifdef OPENMVG_USE_OPENMP
#pragma omp critical
#endif
    {
      stats.tracks_processed += scratch.tracks_processed;
      stats.rejected_border += scratch.rejected_border;
      stats.rejected_low_contrast += scratch.rejected_low_contrast;
      stats.rejected_shift += scratch.rejected_shift;
      stats.rejected_affine_det += scratch.rejected_affine_det;
      stats.rejected_zncc += scratch.rejected_zncc;
      stats.fallback_2dof_count += scratch.fallback_2dof_count;
      all_shifts.insert(all_shifts.end(), scratch.local_shifts.begin(), scratch.local_shifts.end());
    }
  }

  stats.elapsed_phase2_s = timer.elapsed();
  stats.observations_refined = all_shifts.size();

  if (!all_shifts.empty())
  {
    double sum_shift = 0.0;
    float max_shift = 0.0f;
    for (const float s : all_shifts)
    {
      sum_shift += static_cast<double>(s);
      if (s > max_shift)
        max_shift = s;
    }
    stats.mean_shift_px = sum_shift / static_cast<double>(all_shifts.size());
    stats.median_shift_px = static_cast<double>(ComputeMedian(all_shifts));
    stats.max_applied_shift_px = static_cast<double>(max_shift);
  }

  if (out_stats)
    *out_stats = stats;

  return true;
}

} // namespace

MultiViewLSMRefiner::MultiViewLSMRefiner(const MultiViewLSMParams& params)
  : params_(params)
{
}

bool MultiViewLSMRefiner::Refine(
  const SfM_Data& sfm_data,
  const tracks::STLMAPTracks& map_tracks,
  Features_Provider& features_provider,
  MultiViewLSMStats* stats) const
{
  // Per-view image loader: decodes grayscale image into local `owned_storage`, which goes out of
  // scope immediately after Phase 1 copies that view's guard patches.
  auto load_from_disk =
    [&sfm_data](
      const uint32_t view_id,
      const image::Image<unsigned char>*& out_ptr,
      image::Image<unsigned char>& owned_storage) -> bool
  {
    const auto view_it = sfm_data.GetViews().find(view_id);
    if (view_it == sfm_data.GetViews().end())
      return false;

    const std::string img_path =
      stlplus::create_filespec(sfm_data.s_root_path, view_it->second->s_Img_path);
    if (!image::ReadImage(img_path.c_str(), &owned_storage))
    {
      OPENMVG_LOG_WARNING << "MultiViewLSMRefiner: failed to load image: " << img_path;
      return false;
    }
    out_ptr = &owned_storage;
    return true;
  };

  return RunRefinement(params_, map_tracks, features_provider, load_from_disk, stats);
}

bool MultiViewLSMRefiner::RefineFromImages(
  const Hash_Map<IndexT, image::Image<unsigned char>>& images,
  const tracks::STLMAPTracks& map_tracks,
  Features_Provider& features_provider,
  MultiViewLSMStats* stats) const
{
  // Zero-copy view lookup for in-memory grayscale images (used primarily in unit tests).
  auto load_from_memory =
    [&images](
      const uint32_t view_id,
      const image::Image<unsigned char>*& out_ptr,
      image::Image<unsigned char>& /*unused*/) -> bool
  {
    const auto it = images.find(view_id);
    if (it == images.end())
      return false;
    out_ptr = &it->second;
    return true;
  };

  return RunRefinement(params_, map_tracks, features_provider, load_from_memory, stats);
}

std::string MultiViewLSMRefiner::FormatStats(const MultiViewLSMStats& stats)
{
  const double total_obs = static_cast<double>(std::max<size_t>(1, stats.observations_total));
  const size_t total_rejected =
    stats.rejected_border + stats.rejected_low_contrast + stats.rejected_shift +
    stats.rejected_affine_det + stats.rejected_zncc;

  std::ostringstream os;
  os << std::fixed << std::setprecision(3)
     << "\n-------------------------------"
     << "\n-- MV-LSM Refinement Stats ----"
     << "\n-- Tracks total / processed : " << stats.tracks_total << " / " << stats.tracks_processed
     << "\n-- Observations total       : " << stats.observations_total
     << "\n-- Observations refined     : " << stats.observations_refined
     << " (" << (100.0 * static_cast<double>(stats.observations_refined) / total_obs) << "%)"
     << "\n-- Observations rejected    : " << total_rejected
     << " (" << (100.0 * static_cast<double>(total_rejected) / total_obs) << "%)"
     << "\n     - border               : " << stats.rejected_border
     << "\n     - low contrast         : " << stats.rejected_low_contrast
     << "\n     - max shift            : " << stats.rejected_shift
     << "\n     - affine det           : " << stats.rejected_affine_det
     << "\n     - min ZNCC             : " << stats.rejected_zncc
     << "\n-- 2-DoF aperture fallbacks : " << stats.fallback_2dof_count
     << "\n-- Applied shift (px)       : mean=" << stats.mean_shift_px
     << ", median=" << stats.median_shift_px
     << ", max=" << stats.max_applied_shift_px
     << "\n-- Elapsed (s)              : phase1=" << stats.elapsed_phase1_s
     << ", phase2=" << stats.elapsed_phase2_s
     << "\n-------------------------------";
  return os.str();
}

} // namespace sfm
} // namespace openMVG
