#include "SFOPParams.hpp"

namespace openMVG
{
namespace features
{
namespace sfop
{

/**
   * @brief Construct a new SFOPParams object
   * 
   * @param nbOctave    Number of octave 
   * @param nbSlice     Number of slice per octave
   * @param s           Standard deviation of the ideal image model noise 
   * @param lambdaW     Weight on the lambda 
   * @param precisionTh Precision threshold used to filter some keypoints 
   */
SFOPParams::SFOPParams( const int   nbOctave,
                        const int   nbSlice,
                        const float s,
                        const float lambdaW,
                        const float precisionTh )
    : m_nb_octave( nbOctave ),
      m_nb_slice( nbSlice ),
      m_s( s ),
      m_lambda_w( lambdaW ),
      m_precision_th( precisionTh )
{
}

/**
   * @brief Get number of octave 
   * 
   * @return Number of octave 
   */
int SFOPParams::nbOctave( void ) const
{
  return m_nb_octave;
}

/**
   * @brief Set the number of octave 
   * 
   * @param nbOct New number of octave 
   */
void SFOPParams::setNbOctave( const int nbOct )
{
  m_nb_octave = nbOct;
}

/**
   * @brief Get number of slice per octave 
   * 
   * @return Number of slice per octave 
   */
int SFOPParams::nbSlice( void ) const
{
  return m_nb_slice;
}

/**
    * @brief Set the Nb Slice object
    * 
    * @param nbSlice New number of slice per octave 
    */
void SFOPParams::setNbSlice( const int nbSlice )
{
  m_nb_slice = nbSlice;
}

/**
   * @brief Get standard deviation of the noise of the ideal image model 
   * 
   * @return Standard deviation of the noise of the image  
   */
float SFOPParams::noiseStandardDeviation( void ) const
{
  return m_s;
}

/**
   * @brief Set the Noise Standard Deviation object
   * 
   * @param s new standard deviation of the noise image 
   */
void SFOPParams::setNoiseStandardDeviation( const float s )
{
  m_s = s;
}

/**
   * @brief Get current weighting factor for lambda values 
   * 
   * @return current weighting of the lambdas
   */
float SFOPParams::lambdaWeight( void ) const
{
  return m_lambda_w;
}

/**
   * @brief Set the Lambda Weight 
   * 
   * @param lw New lambda weight 
   */
void SFOPParams::setLambdaWeight( const float lw )
{
  m_lambda_w = lw;
}

/**
   * @brief Get the precision threshold for filtering 
   * 
   * @return Current precision threshold 
   */
float SFOPParams::precisionThreshold( void ) const
{
  return m_precision_th;
}

/**
   * @brief Set the Precision Threshold 
   * 
   * @param precTh New precision threshold 
   */
void SFOPParams::setPrecisionThreshold( const float precTh )
{
  m_precision_th = precTh;
}

} // namespace sfop
} // namespace features
} // namespace openMVG
