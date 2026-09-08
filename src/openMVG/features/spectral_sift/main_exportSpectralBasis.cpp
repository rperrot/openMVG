// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/spectral_sift/spectral_sift_basis.hpp"
#include "openMVG/system/logger.hpp"
#include "third_party/cmdLine/cmdLine.h"

#include <cstdlib>
#include <string>

using namespace openMVG;
using namespace openMVG::features::spectral_sift;

int main( int argc, char** argv )
{
  CmdLine cmd;

  float       sigma_min        = 1.6f;
  float       sigma_max        = 3.2f;
  int         degree           = 3;
  float       energy_threshold = 0.999f;
  std::string filter_type_str  = "SLOG";
  std::string output_filename  = "";

  cmd.add( make_option( 'm', sigma_min, "sigma_min" ) );
  cmd.add( make_option( 'M', sigma_max, "sigma_max" ) );
  cmd.add( make_option( 'd', degree, "degree" ) );
  cmd.add( make_option( 'e', energy_threshold, "energy" ) );
  cmd.add( make_option( 't', filter_type_str, "type" ) );
  cmd.add( make_option( 'o', output_filename, "output" ) );

  try
  {
    if ( argc == 1 )
      throw std::string( "Invalid command line parameter." );
    cmd.process( argc, argv );
  }
  catch ( const std::string& s )
  {
    OPENMVG_LOG_INFO
        << "Usage: " << argv[ 0 ] << "\n"
        << "[-o|--output] Output JSON filename (required)\n"
        << "[-m|--sigma_min] Minimum sigma (default: 1.6)\n"
        << "[-M|--sigma_max] Maximum sigma (default: 3.2)\n"
        << "[-d|--degree] Polynomial degree (3 or 4, default: 3)\n"
        << "[-e|--energy] Target energy ratio threshold (default: 0.999)\n"
        << "[-t|--type] Filter type (SLOG or GAUSSIAN, default: SLOG)\n";
    return EXIT_FAILURE;
  }

  if ( output_filename.empty() )
  {
    OPENMVG_LOG_ERROR << "Output filename is required.";
    return EXIT_FAILURE;
  }

  if ( degree != 3 && degree != 4 )
  {
    OPENMVG_LOG_ERROR << "Only polynomial degrees 3 and 4 are supported.";
    return EXIT_FAILURE;
  }

  ESpectralFilterType filter_type = FILTER_SLOG;
  if ( filter_type_str == "GAUSSIAN" || filter_type_str == "gaussian" )
  {
    filter_type = FILTER_GAUSSIAN;
  }

  OPENMVG_LOG_INFO
      << "Computing Spectral Basis:\n"
      << "  - Type: " << ( ( filter_type == FILTER_SLOG ) ? "sLoG" : "Gaussian" ) << "\n"
      << "  - Sigma range: [" << sigma_min << ", " << sigma_max << "]\n"
      << "  - Degree: " << degree << "\n"
      << "  - Target energy ratio: " << energy_threshold;

  SpectralBasis basis;
  if ( !ComputeSpectralBasis( sigma_min, sigma_max, degree, energy_threshold, filter_type, basis ) )
  {
    OPENMVG_LOG_ERROR << "Failed to compute spectral basis.";
    return EXIT_FAILURE;
  }

  OPENMVG_LOG_INFO
      << "Spectral Basis computed successfully:\n"
      << "  - Number of bases (K): " << basis.GetK() << "\n"
      << "  - Achieved energy ratio: " << basis.GetEnergyRatioAchieved() * 100.0f << " %\n"
      << "  - Kernel size: " << basis.GetKernelSize() << "x" << basis.GetKernelSize() << "\n"
      << "  - Exporting to: " << output_filename;

  if ( !SaveSpectralBasis( output_filename, basis ) )
  {
    OPENMVG_LOG_ERROR << "Failed to save spectral basis to: " << output_filename;
    return EXIT_FAILURE;
  }

  OPENMVG_LOG_INFO << "Export completed successfully.";
  return EXIT_SUCCESS;
}
