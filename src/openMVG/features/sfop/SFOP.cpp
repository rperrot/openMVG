#include "SFOP.hpp"

#include "openMVG/features/sfop/SFOPOctave.hpp"
#include "openMVG/features/sfop/SFOPUtilNumeric.hpp"

#include "openMVG/image/image_filtering.hpp"
#include "openMVG/image/image_resampling.hpp"

#include "openMVG/features/sfop/SFOPDebug.hpp"

namespace openMVG
{
namespace features
{
namespace sfop
{

SFOP::SFOP( const SFOPParams& params )
    : m_params( params )
{
}

bool operator<( const SFOPFeature& left, const SFOPFeature& right )
{
  return left.m_precision > right.m_precision;
}

/**
   * @brief Detect features on a given image 
   * 
   * @param img     Image to use 
   * @param kpts    Keypoints of the image 
   */
void SFOP::detect( const image::Image<float>& img, std::vector<SFOPFeature>& kpts ) const
{
  image::Image<float> current = img;

  // process octaves
  for ( int idOctave = 0; idOctave < m_params.nbOctave(); ++idOctave )
  {
    // build scale space
    SFOPOctave l_octave( current, idOctave, m_params.nbSlice() );
    l_octave.buildSlices();

    // detect new features
    l_octave.detect( kpts );

    // Prepare for next octave
    if ( ( idOctave + 1 ) != m_params.nbOctave() )
    {
      image::Image<float> nextImg;
      image::ImageGaussianFilter( current, 0.5, nextImg );
      image::ImageDecimate( nextImg, current );
    }
  }

  // filter features (remove spurious points)
  filterFeatures( kpts, img.Width(), img.Height() );

  // non-maxima suppression
  nonMaximaSuppression( kpts );
}

/**
  * @brief Perform filtering on the keypoints (wrt to noise model of the image)
  * 
  * @param feats Features to clear 
  */
void SFOP::filterFeatures( std::vector<SFOPFeature>& feats, const int width, const int height ) const
{
  auto it_feat = feats.begin();

  const float one_third = 1.f / 3.f;

  const float tmp_h = m_params.noiseStandardDeviation() * m_params.noiseStandardDeviation() / ( 16.f * static_cast<float>( M_PI ) );

  while ( it_feat != feats.end() )
  {
    const float sigma = it_feat->m_sigma;
    const float tau   = sigma * one_third;
    const float tau2  = tau * tau;
    const float tau4  = tau2 * tau2;

    const float h         = tmp_h / tau4;
    const float t_lambda2 = h * m_params.lambdaWeight() * chi2inv( 0.999, /* Significance level 2 * N(sigma) -> 2 * (12 * sigma^2 + 1) */ (int)24.0f * sigma * sigma + 2.0f );

    // Filter points with lamda value less than threshold or in border
    if ( ( it_feat->m_lambda2 < t_lambda2 ) ||
         ( sigma > it_feat->m_x ) ||
         ( sigma > it_feat->m_y ) ||
         ( it_feat->m_x > ( width - sigma ) ) ||
         ( it_feat->m_y > ( height - sigma ) ) )
    {
      it_feat = feats.erase( it_feat );
    }
    else
    {
      ++it_feat;
    }
  }
}

/**
  * @brief Suppresses less precise features if very close to each other
  *
  * @params feats Features to filter based on distance 
  */
void SFOP::nonMaximaSuppression( std::vector<SFOPFeature>& feats ) const
{
  std::sort( feats.begin(), feats.end() );

  const float inv_ln2 = 1.f / std::log( 2.f );

  // loop through all features in set
  for ( auto it1 = feats.begin(); it1 != feats.end(); ++it1 )
  {
    // loop through remaining features with smaller precision
    auto it2 = ++it1;
    for ( --it1; it2 != feats.end(); ++it2 )
    {
      // compute Mahalanobis distance (distance in x, y, sigma)
      const float dx              = it1->m_x - it2->m_x;
      const float dy              = it1->m_y - it2->m_y;
      const float meanSigmaSqr    = ( it1->m_sigma * it1->m_sigma ) + ( it2->m_sigma * it2->m_sigma );
      const float invMeanSigmaSqr = 1.f / meanSigmaSqr;
      const float scaleTerm       = 0.5f * m_params.nbSlice() * std::log( it1->m_sigma / it2->m_sigma ) * inv_ln2;
      const float distSqr         = ( dx * dx * invMeanSigmaSqr ) + ( dy * dy * invMeanSigmaSqr ) + scaleTerm * scaleTerm;

      // remove feature if closer than threshold
      if ( distSqr < 1.0f )
      {
        it2 = feats.erase( it2 );
        it2--;
      }
    }
  }
}

} // namespace sfop
} // namespace features
} // namespace openMVG
