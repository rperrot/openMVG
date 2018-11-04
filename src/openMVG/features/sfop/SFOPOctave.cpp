#include "SFOPOctave.hpp"

namespace openMVG
{
namespace features
{
namespace sfop
{

/**
   * @brief Construct a new SFOPOctave 
   * 
   * @param baseImg     First image of the octave 
   * @param idOctave    Id of the octave 
   * @param nb_slice    Number of slice in this octave 
   */
SFOPOctave::SFOPOctave( const image::Image<float>& baseImg,
                        const int                  idOctave,
                        const int                  nb_slice )
    : m_nb_slice( nb_slice ),
      m_id_octave( idOctave ),
      m_base_img( baseImg )
{
}

/**
   * @brief Build all internal slices of the octave
   * 
   */
void SFOPOctave::buildSlices( void )
{
  // Build ( m_nb_slice  + 2 ) slices :
  // from (incl.) id : -1
  // to   (incl.) id : m_nb_slice
  for ( int idSlice = -1; idSlice <= m_nb_slice; ++idSlice )
  {
    m_slices.emplace_back( SFOPSlice( m_base_img, idSlice, m_id_octave, m_nb_slice ) );
  }

  // Todo : make it parallel ?
  for ( auto& it : m_slices )
  {
    it.buildSlice();
  }
}

/**
   * @brief Detect features in this octave 
   * 
   * @param kpts set of keypoints 
   */
void SFOPOctave::detect( std::vector<SFOPFeature>& kpts ) const
{
  // Detect features using three consecutives slices
  for ( int idSlice = +1; idSlice < m_slices.size() - 1; ++idSlice )
  {
    m_slices[ idSlice ].detect( m_slices[ idSlice - 1 ], m_slices[ idSlice + 1 ], kpts );
  }
}

} // namespace sfop
} // namespace features
} // namespace openMVG
