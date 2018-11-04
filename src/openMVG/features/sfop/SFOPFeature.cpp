#include "SFOPFeature.hpp"

namespace openMVG
{
namespace features
{
namespace sfop
{
/**
  * @brief Constructor
  *
  * @param x         x-coordinate
  * @param y         y-coordinate 
  * @param sigma     Size
  * @param idSlice   Id of the slice 
  * @param idOctave  Id of the octave 
  * @param lambda2   Eigenvalue lambda2
  * @param precision Precision
  */
SFOPFeature::SFOPFeature( const float x,
                          const float y,
                          const float sigma,
                          const int   idSlice,
                          const int   idOctave,
                          const float lambda2,
                          const float precision )
    : m_x( x ),
      m_y( y ),
      m_sigma( sigma ),
      m_id_slice( idSlice ),
      m_id_octave( idOctave ),
      m_lambda2( lambda2 ),
      m_precision( precision )
{
}

} // namespace sfop
} // namespace features
} // namespace openMVG
