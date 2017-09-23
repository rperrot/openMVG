#ifndef _OPENMVG_MVS_PUTATIVE_POINT_HPP_
#define _OPENMVG_MVS_PUTATIVE_POINT_HPP_

#include "openMVG/numeric/numeric.h"

#include <tuple>

namespace MVS
{
/**
* @brief Class representing a putative point
*/
class PutativePoint
{
  public:

    /**
    * @brief Constructor
    * @param pos Initial position
    * @param nor Initial normal
    * @param col Initial color
    */
    PutativePoint( const openMVG::Vec3 & pos = openMVG::Vec3( 0.0 , 0.0 , 0.0 ) ,
                   const openMVG::Vec3 & nor = openMVG::Vec3( 0.0 , 0.0 , 0.0 ) ,
                   const openMVG::Vec3 & col = openMVG::Vec3( 0.0 , 0.0 , 0.0 ) ) ;


    /**
    * @brief Copy ctr
    * @param src Source
    */
    PutativePoint( const PutativePoint & src ) = default ;

    /**
    * @brief Move ctr
    * @param src Source
    */
    PutativePoint( PutativePoint && src ) = default ;

    /**
    * @brief Assignment operator
    * @param src Source
    * @return Self after assignement
    */
    PutativePoint & operator=( const PutativePoint & src ) = default ;

    /**
    * @brief Move assignment operator
    * @param src Source
    * @return Self after assignment
    */
    PutativePoint & operator=( PutativePoint && src ) = default ;

    /**
    * @brief Reset values of the point
    * @param pt Point positions
    * @param nor Point normal
    * @param col Point color (in range [0;1]^3)
    */
    void setInitialPoint( const openMVG::Vec3 & pt , const openMVG::Vec3 & nor , const openMVG::Vec3 & col ) ;

    /**
    * @brief Add a new candidate for this point
    * @param pos Position of the candidate
    * @param nor Normal for this candidate
    * @param col Color for this candidate
    */
    void addCandidate( const openMVG::Vec3 & pos , const openMVG::Vec3 & nor , const openMVG::Vec3 & col ) ;

    /**
    * @brief Get final position of this point ad the mean of all candidates
    * @return position,normal,color for the final point
    */
    std::tuple<openMVG::Vec3, openMVG::Vec3, openMVG::Vec3> getPoint( void ) const ;

    /**
    * @brief Get number of candidate to generate this point
    * @return Number of candidate
    */
    unsigned long nbCandidate( void ) const ;

  private:

    openMVG::Vec3 m_cur_pt_sum ;
    openMVG::Vec3 m_cur_nor_sum ;
    openMVG::Vec3 m_cur_col_sum ;

    unsigned long m_nb_cand ;
} ;

} // namespace MVS

#endif