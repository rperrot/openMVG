// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/**
 * @file main_RefineTracksLSM.cpp
 * @brief Standalone CLI executable for Multi-View Least-Squares Matching (MV-LSM) track refinement.
 *
 * @details
 * Designed to run after pairwise geometric filtering (`openMVG_main_GeometricFilter`) and
 * before SfM reconstruction (`openMVG_main_SfM`):
 * 1. Loads `SfM_Data` views/intrinsics, `image_describer.json` region schema, `.feat` features
 *    (with scales & orientations when available), and filtered pairwise matches.
 * 2. Builds conflict-free multi-view feature tracks (`tracks::TracksBuilder`) filtered to length
 *    $\ge \text{min\_track\_length}$.
 * 3. Runs `MultiViewLSMRefiner` directly on full-resolution grayscale images to refine keypoint
 *    $(x, y)$ positions to sub-pixel precision.
 * 4. Writes updated `.feat` files atomically via temporary files, preserving all auxiliary region
 *    attributes (scale, orientation) and leaving `.desc` descriptors untouched.
 */

#include "openMVG/features/feature.hpp"
#include "openMVG/features/regions.hpp"
#include "openMVG/sfm/pipelines/multi_view_lsm_refiner.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_matches_provider.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/system/logger.hpp"
#include "openMVG/system/timer.hpp"
#include "openMVG/tracks/tracks.hpp"

#include "third_party/cmdLine/cmdLine.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

using namespace openMVG;
using namespace openMVG::features;
using namespace openMVG::sfm;

int main(int argc, char** argv)
{
  OPENMVG_LOG_INFO
    << "\n-----------------------------------------------------------"
    << "\n Multi-View Least-Squares Matching (MV-LSM) Track Refinement"
    << "\n-----------------------------------------------------------";

  CmdLine cmd;

  std::string filename_sfm_data;
  std::string directory_match;
  std::string filename_match;
  std::string directory_output;

  MultiViewLSMParams params;
  // Use int (0/1) rather than single-char OptionSwitch so long-only options avoid short-flag collisions
  int use_bicubic = params.use_bicubic ? 1 : 0;
  int use_orientation = params.use_orientation ? 1 : 0;
  int use_scale = params.use_scale ? 1 : 0;

  // I/O options
  cmd.add(make_option('i', filename_sfm_data, "input_file"));
  cmd.add(make_option('m', directory_match, "match_dir"));
  cmd.add(make_option('M', filename_match, "match_file"));
  cmd.add(make_option('o', directory_output, "output_dir"));

  // Optional MV-LSM algorithm parameters (`'\0'` registers long-only `--name` options in cmdLine.h;
  // never invoke `cmd.used('\0')`).
  cmd.add(make_option('\0', params.patch_radius, "patch_radius"));
  cmd.add(make_option('\0', params.guard_radius, "guard_radius"));
  cmd.add(make_option('\0', params.max_iterations, "max_iterations"));
  cmd.add(make_option('\0', params.outer_iterations, "outer_iterations"));
  cmd.add(make_option('\0', params.min_track_length, "min_track_length"));
  cmd.add(make_option('\0', params.coarse_sigma, "coarse_sigma"));
  cmd.add(make_option('\0', params.min_zncc, "min_zncc"));
  cmd.add(make_option('\0', params.max_shift_px, "max_shift_px"));
  cmd.add(make_option('\0', params.affine_reg, "affine_reg"));
  cmd.add(make_option('\0', params.huber_delta, "huber_delta"));
  cmd.add(make_option('\0', params.spatial_sigma, "spatial_sigma"));
  cmd.add(make_option('\0', params.max_affine_cond, "max_affine_cond"));
  cmd.add(make_option('\0', params.det_min, "det_min"));
  cmd.add(make_option('\0', params.det_max, "det_max"));
  cmd.add(make_option('\0', params.min_patch_std, "min_patch_std"));
  cmd.add(make_option('\0', params.convergence_eps, "convergence_eps"));
  cmd.add(make_option('\0', use_bicubic, "use_bicubic"));
  cmd.add(make_option('\0', use_orientation, "use_orientation"));
  cmd.add(make_option('\0', use_scale, "use_scale"));
  cmd.add(make_option('\0', params.max_io_threads, "max_io_threads"));

  try
  {
    if (argc == 1)
      throw std::string("Invalid parameter.");
    cmd.process(argc, argv);
  }
  catch (const std::string& s)
  {
    OPENMVG_LOG_INFO << "Usage: " << argv[0] << '\n'
      << "[Required]\n"
      << "[-i|--input_file] path to a SfM_Data scene (e.g. sfm_data.json)\n"
      << "[-m|--match_dir] path to the directory containing .feat and image_describer.json\n"
      << "    (optional if -M/--match_file points to an existing match file in that directory)\n"
      << "\n[Optional I/O]\n"
      << "[-M|--match_file] path or filename of matches (e.g. matches.f.bin)\n"
      << "[-o|--output_dir] output directory for refined .feat files (default: match_dir, in-place)\n"
      << "\n[Optional MV-LSM parameters]\n"
      << "[--patch_radius] half-width of optimization patch (default: " << params.patch_radius << ")\n"
      << "[--guard_radius] half-width of extracted uint8 guard buffer (default: " << params.guard_radius << ")\n"
      << "[--max_iterations] max IC-LK iterations per outer pass (default: " << params.max_iterations << ")\n"
      << "[--outer_iterations] consensus template update passes per level (default: " << params.outer_iterations << ")\n"
      << "[--min_track_length] minimum track length to refine (default: " << params.min_track_length << ")\n"
      << "[--coarse_sigma] Gaussian sigma for coarse pyramid level, <=0 disables (default: " << params.coarse_sigma << ")\n"
      << "[--min_zncc] minimum final ZNCC to accept observation (default: " << params.min_zncc << ")\n"
      << "[--max_shift_px] maximum accepted shift magnitude in px (default: " << params.max_shift_px << ")\n"
      << "[--affine_reg] Tikhonov regularization on affine parameters (default: " << params.affine_reg << ")\n"
      << "[--huber_delta] Huber threshold on standardized residuals (default: " << params.huber_delta << ")\n"
      << "[--spatial_sigma] spatial Gaussian weighting sigma, <=0=auto (default: " << params.spatial_sigma << ")\n"
      << "[--max_affine_cond] max 4x4 affine condition number before 2-DoF fallback (default: " << params.max_affine_cond << ")\n"
      << "[--det_min] minimum allowed affine warp determinant (default: " << params.det_min << ")\n"
      << "[--det_max] maximum allowed affine warp determinant (default: " << params.det_max << ")\n"
      << "[--min_patch_std] min patch gray-level std to avoid flat regions (default: " << params.min_patch_std << ")\n"
      << "[--convergence_eps] IC-LK translation convergence threshold in px (default: " << params.convergence_eps << ")\n"
      << "[--use_bicubic] 0=bilinear, 1=Keys bicubic interpolation (default: " << use_bicubic << ")\n"
      << "[--use_orientation] 0=off, 1=initialize rotation from feature orientation (default: " << use_orientation << ")\n"
      << "[--use_scale] 0=off, 1=initialize relative scale from feature scale (default: " << use_scale << ")\n"
      << "[--max_io_threads] max OpenMP threads for Phase 1 image decoding (default: " << params.max_io_threads << ")\n";

    OPENMVG_LOG_ERROR << s;
    return EXIT_FAILURE;
  }

  params.use_bicubic = (use_bicubic != 0);
  params.use_orientation = (use_orientation != 0);
  params.use_scale = (use_scale != 0);

  if (filename_sfm_data.empty())
  {
    OPENMVG_LOG_ERROR << "Missing required -i/--input_file parameter.";
    return EXIT_FAILURE;
  }

  // Infer match_dir from match_file when -m is omitted
  if (directory_match.empty() && !filename_match.empty() && stlplus::file_exists(filename_match))
  {
    directory_match = stlplus::folder_part(filename_match);
  }

  if (directory_match.empty())
  {
    OPENMVG_LOG_ERROR << "Missing -m/--match_dir (or valid -M/--match_file path).";
    return EXIT_FAILURE;
  }

  if (directory_output.empty())
  {
    directory_output = directory_match;
  }

  // 1. Load SfM_Data (views + intrinsics needed to resolve image paths)
  SfM_Data sfm_data;
  if (!Load(sfm_data, filename_sfm_data, ESfM_Data(VIEWS | INTRINSICS)))
  {
    OPENMVG_LOG_ERROR << "The input SfM_Data file \"" << filename_sfm_data << "\" cannot be read.";
    return EXIT_FAILURE;
  }

  // 2. Load region schema from image_describer.json
  const std::string sImage_describer = stlplus::create_filespec(directory_match, "image_describer", "json");
  std::unique_ptr<Regions> regions_type = Init_region_type_from_file(sImage_describer);
  if (!regions_type)
  {
    OPENMVG_LOG_ERROR << "Invalid: " << sImage_describer << " regions type file.";
    return EXIT_FAILURE;
  }

  // 3. Load features (positions, plus dominant orientations & scales for geometric initialization)
  std::shared_ptr<Features_Provider> feats_provider = std::make_shared<Features_Provider>();
  if (!feats_provider->load(sfm_data, directory_match, regions_type))
  {
    OPENMVG_LOG_ERROR << "Cannot load features in directory: " << directory_match << ".";
    return EXIT_FAILURE;
  }

  // 4. Load pairwise matches (try explicit path first, then relative to match_dir, then standard defaults)
  std::shared_ptr<Matches_Provider> matches_provider = std::make_shared<Matches_Provider>();
  bool matches_loaded = false;
  if (!filename_match.empty())
  {
    if (stlplus::is_file(filename_match))
      matches_loaded = matches_provider->load(sfm_data, filename_match);
    if (!matches_loaded)
      matches_loaded = matches_provider->load(sfm_data, stlplus::create_filespec(directory_match, filename_match));
  }
  else
  {
    matches_loaded =
      matches_provider->load(sfm_data, stlplus::create_filespec(directory_match, "matches.f.bin")) ||
      matches_provider->load(sfm_data, stlplus::create_filespec(directory_match, "matches.f.txt")) ||
      matches_provider->load(sfm_data, stlplus::create_filespec(directory_match, "matches.e.bin")) ||
      matches_provider->load(sfm_data, stlplus::create_filespec(directory_match, "matches.e.txt"));
  }

  if (!matches_loaded)
  {
    OPENMVG_LOG_ERROR << "Cannot load the match file.";
    return EXIT_FAILURE;
  }

  // 5. Build conflict-free tracks and filter by minimum track length
  openMVG::system::Timer timer;
  tracks::STLMAPTracks map_tracks;
  {
    tracks::TracksBuilder tracksBuilder;
    tracksBuilder.Build(matches_provider->pairWise_matches_);
    tracksBuilder.Filter(static_cast<uint32_t>(params.min_track_length));
    tracksBuilder.ExportToSTL(map_tracks);
    OPENMVG_LOG_INFO << "Filtered tracks (length >= " << params.min_track_length
                     << "): " << map_tracks.size();
  }

  // 6. Execute two-phase MV-LSM refinement directly on grayscale full-resolution images
  MultiViewLSMRefiner refiner(params);
  MultiViewLSMStats stats;
  if (!refiner.Refine(sfm_data, map_tracks, *feats_provider, &stats))
  {
    OPENMVG_LOG_ERROR << "MV-LSM track refinement failed.";
    return EXIT_FAILURE;
  }

  OPENMVG_LOG_INFO << MultiViewLSMRefiner::FormatStats(stats);

  // 7. Prepare output directory and copy image_describer.json when exporting to a separate folder
  if (!stlplus::folder_exists(directory_output))
  {
    if (!stlplus::folder_create(directory_output))
    {
      OPENMVG_LOG_ERROR << "Cannot create output directory: " << directory_output;
      return EXIT_FAILURE;
    }
  }

  if (stlplus::folder_remove_end_separator(directory_output) !=
      stlplus::folder_remove_end_separator(directory_match))
  {
    const std::string out_describer = stlplus::create_filespec(directory_output, "image_describer", "json");
    if (!stlplus::file_copy(sImage_describer, out_describer))
    {
      OPENMVG_LOG_WARNING << "Failed to copy image_describer.json to " << directory_output;
    }
  }

  // 8. Export refined .feat files while preserving auxiliary feature metadata (scale, orientation).
  //    Each view reloads its original .feat into a Regions instance, updates (x, y) via
  //    SetRegionPosition(), writes to a temporary file, and moves atomically into place.
  std::vector<const View*> views_list;
  views_list.reserve(sfm_data.GetViews().size());
  for (const auto& kv : sfm_data.GetViews())
  {
    views_list.push_back(kv.second.get());
  }

  std::atomic<bool> write_ok(true);
#ifdef OPENMVG_USE_OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
  for (int i = 0; i < static_cast<int>(views_list.size()); ++i)
  {
    if (!write_ok.load(std::memory_order_relaxed))
      continue;

    const View* view = views_list[static_cast<size_t>(i)];
    const std::string img_name = stlplus::create_filespec(sfm_data.s_root_path, view->s_Img_path);
    const std::string basename = stlplus::basename_part(img_name);
    const std::string in_feat_file = stlplus::create_filespec(directory_match, basename, ".feat");
    const std::string out_feat_file = stlplus::create_filespec(directory_output, basename, ".feat");
    const std::string tmp_feat_file = out_feat_file + ".lsm_tmp";

    std::unique_ptr<Regions> regions(regions_type->EmptyClone());
    if (!stlplus::file_exists(in_feat_file) || !regions->LoadFeatures(in_feat_file))
    {
      OPENMVG_LOG_ERROR << "Cannot reload feature file: " << in_feat_file;
      write_ok.store(false, std::memory_order_relaxed);
      continue;
    }

    const PointFeatures& refined_feats = feats_provider->getFeatures(view->id_view);
    const size_t count = std::min(regions->RegionCount(), refined_feats.size());
    for (size_t k = 0; k < count; ++k)
    {
      regions->SetRegionPosition(k, refined_feats[k].x(), refined_feats[k].y());
    }

    if (!regions->SaveFeatures(tmp_feat_file))
    {
      OPENMVG_LOG_ERROR << "Failed to write temporary feature file: " << tmp_feat_file;
      stlplus::file_delete(tmp_feat_file);
      write_ok.store(false, std::memory_order_relaxed);
      continue;
    }

    if (stlplus::file_exists(out_feat_file) && !stlplus::file_delete(out_feat_file))
    {
      OPENMVG_LOG_ERROR << "Failed to replace existing feature file: " << out_feat_file;
      stlplus::file_delete(tmp_feat_file);
      write_ok.store(false, std::memory_order_relaxed);
      continue;
    }

    if (!stlplus::file_move(tmp_feat_file, out_feat_file))
    {
      OPENMVG_LOG_ERROR << "Failed to move temporary feature file to: " << out_feat_file;
      stlplus::file_delete(tmp_feat_file);
      write_ok.store(false, std::memory_order_relaxed);
      continue;
    }
  }

  if (!write_ok.load())
  {
    return EXIT_FAILURE;
  }

  OPENMVG_LOG_INFO << "MV-LSM track refinement & .feat export took (s): " << timer.elapsed();
  return EXIT_SUCCESS;
}
