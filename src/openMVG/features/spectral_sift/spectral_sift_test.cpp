// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2026 openMVG authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/features/spectral_sift/spectral_sift_basis.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_convolver.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_descriptor.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_detector.hpp"
#include "openMVG/features/spectral_sift/spectral_sift_image_describer_io.hpp"
#include "openMVG/image/image_container.hpp"
#include "openMVG/image/image_io.hpp"

#include <cereal/archives/json.hpp>
#include <fstream>
#include <sstream>

#include "testing/testing.h"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace openMVG;
using namespace openMVG::image;
using namespace openMVG::features;
using namespace openMVG::features::spectral_sift;

TEST( SpectralBasis, GenerationAndReconstruction_sLoG )
{
  const float sigma_min        = 1.6f;
  const float sigma_max        = 3.2f;
  const int   degree           = 3;
  const float energy_threshold = 0.999f;

  SpectralBasis basis;
  const bool    ok = ComputeSpectralBasis(
      sigma_min,
      sigma_max,
      degree,
      energy_threshold,
      FILTER_SLOG,
      basis );

  EXPECT_TRUE( ok );
  EXPECT_EQ( basis.GetDegree(), 3 );
  EXPECT_EQ( basis.GetFilterType(), FILTER_SLOG );
  EXPECT_TRUE( basis.GetK() >= 3 );
  EXPECT_TRUE( basis.GetK() <= 8 );
  EXPECT_TRUE( basis.GetEnergyRatioAchieved() >= energy_threshold );

  const auto& bases = basis.GetSpatialBases();
  const int   K     = basis.GetK();
  const int   L     = basis.GetKernelSize();

  // Test orthogonality of spatial bases
  for ( int i = 0; i < K; ++i )
  {
    for ( int j = 0; j < K; ++j )
    {
      float dot = 0.0f;
      for ( int y = 0; y < L; ++y )
      {
        for ( int x = 0; x < L; ++x )
        {
          dot += bases[ i ]( y, x ) * bases[ j ]( y, x );
        }
      }
      if ( i == j )
      {
        EXPECT_NEAR( dot, 1.0f, 1e-4f );
      }
      else
      {
        EXPECT_NEAR( dot, 0.0f, 1e-4f );
      }
    }
  }

  // Test reconstruction accuracy at several intermediate scales
  const std::vector<float> test_sigmas = { 1.8f, 2.2f, 2.7f, 3.1f };
  for ( const float s : test_sigmas )
  {
    const Eigen::MatrixXf theoretical   = GenerateScaleNormalizedLoG( L, s );
    const Eigen::MatrixXf reconstructed = basis.ReconstructFilter( s );

    const float rmse = std::sqrt( ( reconstructed - theoretical ).squaredNorm() / static_cast<float>( L * L ) );
    EXPECT_TRUE( rmse < 1e-3f );
  }
}

TEST( SpectralBasis, GenerationAndReconstruction_Gaussian )
{
  const float sigma_min        = 1.6f;
  const float sigma_max        = 3.2f;
  const int   degree           = 3;
  const float energy_threshold = 0.999f;

  SpectralBasis basis;
  const bool    ok = ComputeSpectralBasis(
      sigma_min,
      sigma_max,
      degree,
      energy_threshold,
      FILTER_GAUSSIAN,
      basis );

  EXPECT_TRUE( ok );
  EXPECT_EQ( basis.GetFilterType(), FILTER_GAUSSIAN );
  EXPECT_TRUE( basis.GetK() >= 2 );
  EXPECT_TRUE( basis.GetEnergyRatioAchieved() >= energy_threshold );

  const int                L           = basis.GetKernelSize();
  const std::vector<float> test_sigmas = { 1.7f, 2.4f, 3.0f };
  for ( const float s : test_sigmas )
  {
    const Eigen::MatrixXf theoretical   = GenerateGaussian( L, s );
    const Eigen::MatrixXf reconstructed = basis.ReconstructFilter( s );

    const float rmse = std::sqrt( ( reconstructed - theoretical ).squaredNorm() / static_cast<float>( L * L ) );
    EXPECT_TRUE( rmse < 1e-3f );
  }
}

TEST( SpectralBasis, Degree4_Option )
{
  const float sigma_min        = 1.6f;
  const float sigma_max        = 3.2f;
  const int   degree           = 4;
  const float energy_threshold = 0.999f;

  SpectralBasis basis;
  const bool    ok = ComputeSpectralBasis(
      sigma_min,
      sigma_max,
      degree,
      energy_threshold,
      FILTER_SLOG,
      basis );

  EXPECT_TRUE( ok );
  EXPECT_EQ( basis.GetDegree(), 4 );
  EXPECT_TRUE( basis.GetEnergyRatioAchieved() >= energy_threshold );

  // Test invalid degree (only 3 and 4 allowed)
  SpectralBasis invalid_basis;
  EXPECT_FALSE( ComputeSpectralBasis( sigma_min, sigma_max, 2, energy_threshold, FILTER_SLOG, invalid_basis ) );
  EXPECT_FALSE( ComputeSpectralBasis( sigma_min, sigma_max, 5, energy_threshold, FILTER_SLOG, invalid_basis ) );
}

TEST( SpectralBasis, CerealJSON_Serialization )
{
  SpectralBasis basis_out;
  EXPECT_TRUE( ComputeSpectralBasis( 1.6f, 3.2f, 3, 0.999f, FILTER_SLOG, basis_out ) );

  const std::string tmp_file = "test_spectral_basis.json";
  EXPECT_TRUE( SaveSpectralBasis( tmp_file, basis_out ) );

  SpectralBasis basis_in;
  EXPECT_TRUE( LoadSpectralBasis( tmp_file, basis_in ) );

  EXPECT_EQ( basis_in.GetFilterType(), basis_out.GetFilterType() );
  EXPECT_NEAR( basis_in.GetSigmaMin(), basis_out.GetSigmaMin(), 1e-5f );
  EXPECT_NEAR( basis_in.GetSigmaMax(), basis_out.GetSigmaMax(), 1e-5f );
  EXPECT_EQ( basis_in.GetDegree(), basis_out.GetDegree() );
  EXPECT_EQ( basis_in.GetK(), basis_out.GetK() );
  EXPECT_EQ( basis_in.GetKernelSize(), basis_out.GetKernelSize() );
  EXPECT_NEAR( basis_in.GetEnergyRatioAchieved(), basis_out.GetEnergyRatioAchieved(), 1e-5f );

  // Compare spatial bases and polynomial coefficients
  EXPECT_NEAR( ( basis_in.GetPolyCoeffs() - basis_out.GetPolyCoeffs() ).norm(), 0.0f, 1e-5f );
  for ( int k = 0; k < basis_out.GetK(); ++k )
  {
    EXPECT_NEAR( ( basis_in.GetSpatialBases()[ k ] - basis_out.GetSpatialBases()[ k ] ).norm(), 0.0f, 1e-5f );
  }

  std::remove( tmp_file.c_str() );
}

TEST( SpectralConvolver, ForwardAndInverseFFT2D )
{
  const int       W        = 64;
  const int       H        = 48;
  Eigen::MatrixXf test_img = Eigen::MatrixXf::Random( H, W );

  SpectralConvolver convolver;
  MatrixXcf         spectrum;
  convolver.ForwardFFT2D( test_img, spectrum );

  Eigen::MatrixXf reconstructed;
  convolver.InverseFFT2D( spectrum, reconstructed );

  const float diff = ( reconstructed - test_img ).norm();
  EXPECT_NEAR( diff, 0.0f, 1e-3f );
}

TEST( SpectralConvolver, ImpulseResponseConvolution )
{
  const int W = 64;
  const int H = 64;
  const int L = 15;

  // Create a symmetric Gaussian kernel of size L x L
  const Eigen::MatrixXf kernel = GenerateGaussian( L, 2.0f );

  std::vector<Eigen::MatrixXf> kernels = { kernel };
  SpectralConvolver            convolver;
  convolver.Init( W, H, kernels );

  // Create an image with an impulse at (32, 32)
  Image<float> impulse( W, H );
  impulse.fill( 0.0f );
  impulse( 32, 32 ) = 1.0f;

  Image<float> out_conv;
  convolver.Convolve( impulse, 0, out_conv );

  // Verify that the output image is the kernel centered at (32, 32)
  const int R = L / 2;
  for ( int dy = -R; dy <= R; ++dy )
  {
    for ( int dx = -R; dx <= R; ++dx )
    {
      const float expected = kernel( dy + R, dx + R );
      const float actual   = out_conv( 32 + dy, 32 + dx );
      EXPECT_NEAR( actual, expected, 1e-4f );
    }
  }
}

TEST( SpectralDetector, BlobDetection_Degree3 )
{
  const int    W = 128;
  const int    H = 128;
  Image<float> image( W, H );
  image.fill( 0.0f );

  // Add a synthetic Gaussian blob at (64.0, 64.0) with sigma = 2.4
  const float center_x   = 64.0f;
  const float center_y   = 64.0f;
  const float blob_sigma = 2.4f;
  const float s2_2       = 2.0f * blob_sigma * blob_sigma;

  for ( int y = 0; y < H; ++y )
  {
    const float dy = static_cast<float>( y ) - center_y;
    for ( int x = 0; x < W; ++x )
    {
      const float dx = static_cast<float>( x ) - center_x;
      const float r2 = dx * dx + dy * dy;
      image( y, x )  = 1.0f * std::exp( -r2 / s2_2 );
    }
  }

  SpectralSIFTDetector::Params params;
  params.degree_         = 3;
  params.peak_threshold_ = 0.01f;
  SpectralSIFTDetector detector( params );

  std::vector<SpectralKeypoint> keypoints;
  detector.DetectOctave( image, 0, keypoints );

  EXPECT_TRUE( !keypoints.empty() );
  if ( !keypoints.empty() )
  {
    // Find keypoint closest to center
    int   best_idx  = 0;
    float best_dist = 1e9f;
    for ( size_t i = 0; i < keypoints.size(); ++i )
    {
      const float d = std::hypot( keypoints[ i ].x - center_x, keypoints[ i ].y - center_y );
      if ( d < best_dist )
      {
        best_dist = d;
        best_idx  = static_cast<int>( i );
      }
    }

    // Coordinates should be accurate to subpixel (< 0.2 pixel error)
    EXPECT_NEAR( keypoints[ best_idx ].x, center_x, 0.2f );
    EXPECT_NEAR( keypoints[ best_idx ].y, center_y, 0.2f );
    // Scale should match blob_sigma
    EXPECT_NEAR( keypoints[ best_idx ].sigma, blob_sigma, 0.3f );
  }
}

TEST( SpectralDetector, BlobDetection_Degree4 )
{
  const int    W = 128;
  const int    H = 128;
  Image<float> image( W, H );
  image.fill( 0.0f );

  const float center_x   = 64.0f;
  const float center_y   = 64.0f;
  const float blob_sigma = 2.4f;
  const float s2_2       = 2.0f * blob_sigma * blob_sigma;

  for ( int y = 0; y < H; ++y )
  {
    const float dy = static_cast<float>( y ) - center_y;
    for ( int x = 0; x < W; ++x )
    {
      const float dx = static_cast<float>( x ) - center_x;
      const float r2 = dx * dx + dy * dy;
      image( y, x )  = 1.0f * std::exp( -r2 / s2_2 );
    }
  }

  SpectralSIFTDetector::Params params;
  params.degree_         = 4;
  params.peak_threshold_ = 0.01f;
  SpectralSIFTDetector detector( params );

  std::vector<SpectralKeypoint> keypoints;
  detector.DetectOctave( image, 0, keypoints );

  EXPECT_TRUE( !keypoints.empty() );
  if ( !keypoints.empty() )
  {
    int   best_idx  = 0;
    float best_dist = 1e9f;
    for ( size_t i = 0; i < keypoints.size(); ++i )
    {
      const float d = std::hypot( keypoints[ i ].x - center_x, keypoints[ i ].y - center_y );
      if ( d < best_dist )
      {
        best_dist = d;
        best_idx  = static_cast<int>( i );
      }
    }

    EXPECT_NEAR( keypoints[ best_idx ].x, center_x, 0.2f );
    EXPECT_NEAR( keypoints[ best_idx ].y, center_y, 0.2f );
    EXPECT_NEAR( keypoints[ best_idx ].sigma, blob_sigma, 0.3f );
  }
}

TEST( SpectralDescriptor, OrientationAndDescriptorComputation )
{
  const int    W = 128;
  const int    H = 128;
  Image<float> image( W, H );
  image.fill( 0.0f );

  // Synthesize an asymmetric gradient pattern
  for ( int y = 0; y < H; ++y )
  {
    for ( int x = 0; x < W; ++x )
    {
      image( y, x ) = static_cast<float>( x + 2 * y ) / static_cast<float>( W + 2 * H );
    }
  }

  SpectralSIFTDescriptorExtractor extractor;
  EXPECT_TRUE( extractor.Init() );

  SpectralKeypoint kp;
  kp.x      = 64.0f;
  kp.y      = 64.0f;
  kp.sigma  = 2.0f;
  kp.val    = 0.5f;
  kp.octave = 0;

  std::vector<SpectralKeypoint>         in_kps = { kp };
  std::vector<SpectralOrientedKeypoint> out_kps;

  extractor.DescribeOctave( image, 0, in_kps, out_kps );

  EXPECT_TRUE( !out_kps.empty() );
  if ( !out_kps.empty() )
  {
    // Check descriptor norm is non-zero
    EXPECT_TRUE( out_kps[ 0 ].descr.norm() > 0.0f );
    // Check orientation in [0, 2*pi)
    EXPECT_TRUE( out_kps[ 0 ].theta >= 0.0f && out_kps[ 0 ].theta < 2.0f * M_PI );
  }
}

TEST( SpectralSIFT, EmptyImage )
{
  Image<unsigned char>          empty_image;
  Spectral_SIFT_Image_describer describer;
  auto                          regions = describer.Describe( empty_image );
  EXPECT_TRUE( regions != nullptr );
  EXPECT_EQ( regions->RegionCount(), 0 );
}

TEST( SpectralSIFT, CerealPolymorphicSerialization )
{
  std::unique_ptr<Image_describer> describer(
      new Spectral_SIFT_Image_describer(
          Spectral_SIFT_Image_describer::Params(
              0, 5, 12.0f, 0.03f, 3, 1.6f, 3.2f, 0.999f, true ) ) );

  std::stringstream ss;
  {
    cereal::JSONOutputArchive oa( ss );
    oa( cereal::make_nvp( "image_describer", describer ) );
  }

  std::unique_ptr<Image_describer> loaded_describer;
  {
    cereal::JSONInputArchive ia( ss );
    ia( cereal::make_nvp( "image_describer", loaded_describer ) );
  }

  EXPECT_TRUE( loaded_describer != nullptr );
  auto spectral_ptr = dynamic_cast<Spectral_SIFT_Image_describer*>( loaded_describer.get() );
  EXPECT_TRUE( spectral_ptr != nullptr );
  if ( spectral_ptr )
  {
    EXPECT_EQ( spectral_ptr->GetParams().num_octaves_, 5 );
    EXPECT_NEAR( spectral_ptr->GetParams().edge_threshold_, 12.0f, 1e-4f );
    EXPECT_NEAR( spectral_ptr->GetParams().peak_threshold_, 0.03f, 1e-4f );
    EXPECT_EQ( spectral_ptr->GetParams().degree_, 3 );
    EXPECT_TRUE( spectral_ptr->GetParams().root_sift_ );
    EXPECT_NEAR( spectral_ptr->GetParams().tikhonov_lambda_, 1e-3f, 1e-6f );
    EXPECT_NEAR( spectral_ptr->GetParams().margin_factor_, 1.41421356f, 1e-4f );
  }
}

TEST( SpectralSIFT, InterOctaveGridAlignment )
{
  // Synthesize an image containing a Gaussian blob at scale sigma = 4.8.
  // In octave 0 ([1.6, 3.2]), this blob is outside the nominal scale range.
  // In octave 1 (decimated by 2), the effective scale is 4.8 / 2 = 2.4, which is detected.
  // We verify that the reconstructed coordinates (scaled by 2^1) align subpixel-accurately
  // with the ground truth center, proving that ImageDecimate introduces 0 systematic phase shift.
  const int   W        = 256;
  const int   H        = 256;
  const float center_x = 128.0f;
  const float center_y = 128.0f;
  const float sigma    = 4.8f;
  const float s2_2     = 2.0f * sigma * sigma;

  Image<unsigned char> img( W, H );
  for ( int y = 0; y < H; ++y )
  {
    const float dy = static_cast<float>( y ) - center_y;
    for ( int x = 0; x < W; ++x )
    {
      const float dx = static_cast<float>( x ) - center_x;
      const float r2 = dx * dx + dy * dy;
      const float val = 255.0f * std::exp( -r2 / s2_2 );
      img( y, x ) = static_cast<unsigned char>( std::max( 0.0f, std::min( 255.0f, val ) ) );
    }
  }

  Spectral_SIFT_Image_describer describer;
  describer.ChangeParams().num_octaves_    = 2;
  describer.ChangeParams().peak_threshold_ = 0.01f;

  auto regions = describer.Describe( img );
  EXPECT_TRUE( regions != nullptr );
  EXPECT_TRUE( regions->RegionCount() > 0 );

  if ( regions && regions->RegionCount() > 0 )
  {
    const auto* sift_regions = dynamic_cast<const SIFT_Regions*>( regions.get() );
    EXPECT_TRUE( sift_regions != nullptr );
    if ( sift_regions )
    {
      const auto& feats = sift_regions->Features();
      float best_dist = 1e9f;
      int   best_idx  = 0;
      for ( size_t i = 0; i < feats.size(); ++i )
      {
        const float d = std::hypot( feats[ i ].x() - center_x, feats[ i ].y() - center_y );
        if ( d < best_dist )
        {
          best_dist = d;
          best_idx  = static_cast<int>( i );
        }
      }

      // Feature detected in octave 1 accurately matches ground truth position (no systematic +0.5 shift)
      EXPECT_NEAR( feats[ best_idx ].x(), center_x, 0.3f );
      EXPECT_NEAR( feats[ best_idx ].y(), center_y, 0.3f );
      // Effective scale accounts for the pre-smoothing sigma=3.2 applied before decimation:
      // sigma_eff = sqrt(sigma_blob^2 + sigma_smooth^2) = sqrt(4.8^2 + 3.2^2) ~ 5.77
      const float expected_scale = std::hypot( sigma, 3.2f );
      EXPECT_NEAR( feats[ best_idx ].scale(), expected_scale, 0.5f );
    }
  }
}

TEST( SpectralSIFT, DescribeRealImage )
{
  const std::string png_filename = std::string( THIS_SOURCE_DIR ) + "/../../../openMVG_Samples/imageData/StanfordMobileVisualSearch/Ace_0.png";

  Image<unsigned char> in_image;
  EXPECT_TRUE( ReadImage( png_filename.c_str(), &in_image ) );

  Spectral_SIFT_Image_describer describer;
  // Use 2 octaves for quick test execution
  describer.ChangeParams().num_octaves_    = 2;
  describer.ChangeParams().peak_threshold_ = 0.04f;

  auto regions = describer.Describe( in_image );
  EXPECT_TRUE( regions != nullptr );
  EXPECT_TRUE( regions->RegionCount() > 0 );

  // Check that features and descriptors are valid
  const auto* sift_regions = dynamic_cast<const SIFT_Regions*>( regions.get() );
  EXPECT_TRUE( sift_regions != nullptr );
  if ( sift_regions )
  {
    EXPECT_EQ( sift_regions->RegionCount(), sift_regions->Features().size() );
    EXPECT_EQ( sift_regions->RegionCount(), sift_regions->Descriptors().size() );
  }
}

/* ************************************************************************* */
int main()
{
  TestResult tr;
  return TestRegistry::runAllTests( tr );
}
/* ************************************************************************* */
