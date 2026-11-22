// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/**
 * @file multi_view_lsm_refiner.hpp
 * @brief Multi-View Least-Squares Matching (MV-LSM) sub-pixel track refinement.
 *
 * @details
 * Independent per-view keypoint detection produces feature drift across long tracks
 * and perspective bias on oblique camera views ($35^\circ$--$55^\circ$), which
 * degrades Bundle Adjustment reprojection RMSE. This module performs pre-SfM
 * multi-view sub-pixel refinement directly on full-resolution image intensities
 * over filtered feature tracks (`tracks::STLMAPTracks`), combining three ideas:
 *
 * 1. **Track-based pre-SfM keypoint adjustment** (Dusmanu et al., CVPR 2020
 *    *Multi-View Optimization of Local Feature Geometry*): refines 2D observations
 *    within each track prior to triangulation or Bundle Adjustment.
 * 2. **$O(M)$ consensus template alignment & coarse-to-fine pyramid** (Lindenberger
 *    et al., ICCV 2021 *Pixel-Perfect Structure-from-Motion with Featuremetric
 *    Refinement*): instead of quadratic $O(M^2)$ pairwise matching or aligning to
 *    a biased reference image, all $M$ observations $i \in \{1,\dots,M\}$ of a
 *    track align to an iteratively updated consensus template $T(\mathbf{u})$.
 * 3. **6-DoF Inverse-Compositional Lucas-Kanade (IC-LK) affine LSM** (Gruen 1985;
 *    Baltsavias 1991; Baker & Matthews 2004 *Lucas-Kanade 20 Years On*):
 *    each view $i$ models its local patch warp as:
 *    $$W_i(\mathbf{u}; \mathbf{t}_i, M_i) = \mathbf{x}_i^{(0)} + \mathbf{t}_i + L_i^{(0)} M_i \mathbf{u},$$
 *    where $\mathbf{x}_i^{(0)}$ is the initial keypoint position,
 *    $L_i^{(0)} = \alpha_i^{(0)} R(\theta_i)$ initializes relative scale
 *    $\alpha_i^{(0)} = s_i / s_{\text{ref}}$ and dominant orientation $\theta_i$,
 *    $M_i = I_2 + A_i$ captures residual oblique affine shear/foreshortening, and
 *    $\mathbf{t}_i = (\Delta x_i, \Delta y_i)^T$ is the sub-pixel position shift.
 *
 * Stability & memory guarantees for 20--50 Mpx imagery:
 * - Two-phase OpenMP execution (single-pass per-view uint8 guard extraction, then
 *   zero-allocation per-track optimization).
 * - Standardized patches ($\hat{I} = (I - \mu)/\sigma$) for linear gain/offset
 *   invariance, centered spatial Gaussian weights, and Huber robust IRLS weights
 *   to handle partial occlusions and depth discontinuities.
 * - Tikhonov regularization $\lambda_{\text{reg}}\|A_i\|_F^2$ on the affine block
 *   plus automatic $4\times 4$ condition-number fallback to truncated 2-DoF
 *   translation along 1D edges (aperture problem mitigation).
 * - Atomic per-observation rejection gates (image bounds, contrast, max shift,
 *   affine determinant $\det(M_i) \in [\det_{\min}, \det_{\max}]$, and final ZNCC).
 */

#ifndef OPENMVG_SFM_PIPELINES_MULTI_VIEW_LSM_REFINER_HPP
#define OPENMVG_SFM_PIPELINES_MULTI_VIEW_LSM_REFINER_HPP

#include <cstddef>
#include <string>

#include "openMVG/image/image_container.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/tracks/tracks.hpp"
#include "openMVG/types.hpp"

namespace openMVG {
namespace sfm {

/**
 * @brief Configuration parameters for Multi-View Least-Squares Matching (MV-LSM)
 *        sub-pixel track refinement.
 */
struct MultiViewLSMParams
{
  /// Half-width $R$ (pixels) of the optimization window in canonical template coordinates.
  /// The evaluated patch has $(2R + 1) \times (2R + 1)$ pixels (default $R=6 \Rightarrow 13 \times 13 = 169$ px).
  int patch_radius = 6;

  /// Half-width $G$ (pixels) of the raw `uint8_t` guard buffer extracted per observation.
  /// Default $G=15$ ($31 \times 31$, 961 B/obs $\approx 9.6$ MB per 10k observations) accommodates
  /// $\pm 2.5\text{ px}$ shifts, arbitrary rotations ($\sqrt{2}R$), and oblique foreshortening.
  int guard_radius = 15;

  /// Maximum inner Inverse-Compositional Lucas-Kanade (IC-LK) Gauss-Newton iterations
  /// executed per observation during each outer consensus pass.
  int max_iterations = 10;

  /// Number of outer consensus passes (align all views to $T$, center gauge, update $T$)
  /// executed at each pyramid level.
  int outer_iterations = 3;

  /// Minimum track length $M_{\min}$ required to enter refinement (tracks shorter than this are untouched).
  int min_track_length = 3;

  /// Gaussian smoothing standard deviation $\sigma_{\text{coarse}}$ (px) for the coarse pyramid level
  /// to widen the basin of attraction ($\sim 2.5\text{ px}$). Values $\le 0$ disable the coarse pass.
  float coarse_sigma = 1.5f;

  /// Minimum final Zero-Normalized Cross-Correlation $\text{ZNCC}(\hat{I}_i, T) \in [-1, 1]$
  /// required by the atomic acceptance gate to commit the refined position.
  float min_zncc = 0.75f;

  /// Maximum accepted total displacement $\|\mathbf{t}_i\|_2$ (px) from initial position $\mathbf{x}_i^{(0)}$.
  float max_shift_px = 2.5f;

  /// Tikhonov regularization weight $\lambda_{\text{reg}}$ pulling residual affine distortion
  /// $A_i = M_i - I_2$ toward zero: $E_{\text{reg}} = \frac{1}{2}\lambda_{\text{reg}}\|A_i\|_F^2$.
  float affine_reg = 0.05f;

  /// Huber robust loss cutoff $\delta$ on standardized radiometric residuals $r(\mathbf{u}) = \hat{I}_i - T$.
  /// Residuals with $|r| > \delta$ are downweighted by $\delta / |r|$.
  float huber_delta = 0.5f;

  /// Spatial Gaussian window standard deviation $\sigma_s$ (px) over template coordinates $\mathbf{u}$.
  /// Values $\le 0$ automatically use $\sigma_s = 0.5 R$ (3.0 px for $R=6$).
  float spatial_sigma = 0.0f;

  /// Maximum allowed condition number $\kappa(H_{\text{aff}}) = \lambda_{\max}/\lambda_{\min}$ of the $4 \times 4$
  /// affine Hessian block before switching to truncated 2-DoF translation (aperture protection).
  float max_affine_cond = 1e4f;

  /// Minimum allowed residual affine warp determinant $\det(M_i) = \det(I_2 + A_i)$ at final validation.
  float det_min = 0.5f;

  /// Maximum allowed residual affine warp determinant $\det(M_i) = \det(I_2 + A_i)$ at final validation.
  float det_max = 2.0f;

  /// Minimum raw patch intensity standard deviation (gray levels in $[0, 255]$) required to reject
  /// flat/saturated regions (e.g. uniform sky or overexposed surfaces).
  float min_patch_std = 3.0f;

  /// Convergence tolerance (px) on incremental translation $\|\Delta\mathbf{t}\|_2$
  /// (and $0.1 \times$ tolerance on $\|\Delta A\|_F$).
  float convergence_eps = 1e-3f;

  /// When `true`, sample patches with Keys cubic convolution ($a = -0.5$); when `false`, use bilinear.
  bool use_bicubic = false;

  /// When `true`, initialize warp rotation $R(\theta_i)$ from feature orientation $\theta_i$.
  bool use_orientation = true;

  /// When `true`, initialize isotropic warp scale $\alpha_i^{(0)} = s_i / s_{\text{ref}}$ from feature scale ratios.
  bool use_scale = true;

  /// Maximum concurrent OpenMP worker threads during Phase 1 disk I/O + image decoding,
  /// bounding peak uncompressed RAM on 20--50 Mpx images while Phase 2 uses all threads.
  int max_io_threads = 4;
};

/**
 * @brief Execution and rejection statistics reported by `MultiViewLSMRefiner`.
 */
struct MultiViewLSMStats
{
  size_t tracks_total = 0;           ///< Total number of input tracks in `map_tracks`
  size_t tracks_processed = 0;       ///< Tracks meeting `min_track_length` with $\ge 2$ valid patches
  size_t observations_total = 0;     ///< Total observations inside eligible tracks
  size_t observations_refined = 0;   ///< Observations that passed all validation gates and were updated
  size_t rejected_border = 0;        ///< Rejected due to guard or warped patch exiting image/guard bounds
  size_t rejected_low_contrast = 0;  ///< Rejected due to insufficient gray-level variance (`< min_patch_std`)
  size_t rejected_shift = 0;         ///< Rejected due to non-finite state or $\|\mathbf{t}_i\|_2 > \text{max\_shift\_px}$
  size_t rejected_affine_det = 0;    ///< Rejected due to $\det(M_i) \notin [\text{det\_min}, \text{det\_max}]$
  size_t rejected_zncc = 0;          ///< Rejected due to final $\text{ZNCC}(\hat{I}_i, T) < \text{min\_zncc}$
  size_t fallback_2dof_count = 0;    ///< Observations that triggered 2-DoF aperture fallback on the fine level
  double mean_shift_px = 0.0;        ///< Mean applied sub-pixel correction $\|\mathbf{t}_i\|_2$ (px)
  double median_shift_px = 0.0;      ///< Median applied sub-pixel correction (px)
  double max_applied_shift_px = 0.0; ///< Maximum accepted correction magnitude (px)
  double elapsed_phase1_s = 0.0;     ///< Wall time for Phase 1 per-view image load & guard extraction (s)
  double elapsed_phase2_s = 0.0;     ///< Wall time for Phase 2 multi-view track optimization (s)
};

/**
 * @brief Multi-View Least-Squares Matching (MV-LSM) sub-pixel feature refiner.
 *
 * @details
 * Operates in two OpenMP-parallelized phases so 20--50 Mpx datasets never retain
 * full-resolution images simultaneously in RAM or re-read images per track:
 *  - **Phase 1 (per-view)**: Reads each active view once in grayscale, copies contiguous
 *    `uint8_t` guard patches centered on $\text{round}(\mathbf{x}_i^{(0)})$, and frees the image.
 *  - **Phase 2 (per-track)**: Runs coarse-to-fine IC-LK Gauss-Newton optimization with
 *    pre-allocated per-thread scratch buffers and fixed-size Eigen stack matrices, updating
 *    `features_provider.feats_per_view` lock-free when all atomic gates pass.
 */
class MultiViewLSMRefiner
{
public:
  /**
   * @brief Construct a refiner with the specified MV-LSM configuration.
   */
  explicit MultiViewLSMRefiner(const MultiViewLSMParams& params = MultiViewLSMParams());

  /// Return the active configuration parameters.
  const MultiViewLSMParams& Params() const { return params_; }

  /**
   * @brief Refine feature positions across filtered tracks by loading images from `sfm_data`.
   * @param[in]     sfm_data          Scene views & root path used to locate image files.
   * @param[in]     map_tracks        Conflict-free tracks (`TracksBuilder::Filter()` output).
   * @param[in,out] features_provider Features (positions, plus optional orientations/scales) updated in place.
   * @param[out]    stats             Optional summary counters and timings.
   * @return `true` on success, `false` on invalid parameters or conflicting duplicate track nodes.
   */
  bool Refine(
    const SfM_Data& sfm_data,
    const tracks::STLMAPTracks& map_tracks,
    Features_Provider& features_provider,
    MultiViewLSMStats* stats = nullptr) const;

  /**
   * @brief Refine feature positions using preloaded grayscale images in memory (no disk I/O).
   * @param[in]     images            Map from `view_id` to grayscale `Image<unsigned char>`.
   * @param[in]     map_tracks        Conflict-free tracks (`TracksBuilder::Filter()` output).
   * @param[in,out] features_provider Features updated in place.
   * @param[out]    stats             Optional summary counters and timings.
   * @return `true` on success, `false` on invalid parameters or conflicting duplicate track nodes.
   */
  bool RefineFromImages(
    const Hash_Map<IndexT, image::Image<unsigned char>>& images,
    const tracks::STLMAPTracks& map_tracks,
    Features_Provider& features_provider,
    MultiViewLSMStats* stats = nullptr) const;

  /**
   * @brief Format refinement statistics into a human-readable multi-line report string.
   */
  static std::string FormatStats(const MultiViewLSMStats& stats);

private:
  MultiViewLSMParams params_;
};

} // namespace sfm
} // namespace openMVG

#endif // OPENMVG_SFM_PIPELINES_MULTI_VIEW_LSM_REFINER_HPP
