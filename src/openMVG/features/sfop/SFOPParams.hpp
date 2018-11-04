#ifndef OPENMVG_FEATURES_SFOP_SFOP_PARAMS_HPP
#define OPENMVG_FEATURES_SFOP_SFOP_PARAMS_HPP

namespace openMVG
{
namespace features
{
namespace sfop
{

/**
  * @brief Parameters for SFOP 
  * 
  */
struct SFOPParams
{
public:
  /**
   * @brief Construct a new SFOPParams object
   * 
   * @param nbOctave    Number of octave 
   * @param nbSlice     Number of slice per octave
   * @param s           Standard deviation of the ideal image model noise 
   * @param lambdaW     Weight on the lambda 
   * @param precisionTh Precision threshold used to filter some keypoints 
   */
  SFOPParams( const int   nbOctave    = 3,
              const int   nbSlice     = 4,
              const float s           = 0.02f,
              const float lambdaW     = 2.0f,
              const float precisionTh = 0.0f );

  SFOPParams( const SFOPParams& src ) = default;
  SFOPParams( SFOPParams&& src )      = default;

  SFOPParams& operator=( const SFOPParams& src ) = default;
  SFOPParams& operator=( SFOPParams&& src ) = default;

  /**
   * @brief Get number of octave 
   * 
   * @return Number of octave 
   */
  int nbOctave( void ) const;

  /**
   * @brief Set the number of octave 
   * 
   * @param nbOct New number of octave 
   */
  void setNbOctave( const int nbOct );

  /**
   * @brief Get number of slice per octave 
   * 
   * @return Number of slice per octave 
   */
  int nbSlice( void ) const;

  /**
    * @brief Set the Nb Slice object
    * 
    * @param nbSlice New number of slice per octave 
    */
  void setNbSlice( const int nbSlice );

  /**
   * @brief Get standard deviation of the noise of the ideal image model 
   * 
   * @return Standard deviation of the noise of the image  
   */
  float noiseStandardDeviation( void ) const;

  /**
   * @brief Set the Noise Standard Deviation object
   * 
   * @param s new standard deviation of the noise image 
   */
  void setNoiseStandardDeviation( const float s );

  /**
   * @brief Get current weighting factor for lambda values 
   * 
   * @return current weighting of the lambdas
   */
  float lambdaWeight( void ) const;

  /**
   * @brief Set the Lambda Weight 
   * 
   * @param lw New lambda weight 
   */
  void setLambdaWeight( const float lw );

  /**
   * @brief Get the precision threshold for filtering 
   * 
   * @return Current precision threshold 
   */
  float precisionThreshold( void ) const;

  /**
   * @brief Set the Precision Threshold 
   * 
   * @param precTh New precision threshold 
   */
  void setPrecisionThreshold( const float precTh );

  template <class Archive>
  void serialize( Archive& ar );

private:
  int   m_nb_octave;
  int   m_nb_slice;
  float m_s;
  float m_lambda_w;
  float m_precision_th;
};

} // namespace sfop
} // namespace features
} // namespace openMVG

#endif