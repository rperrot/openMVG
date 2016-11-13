#include "PutativePoint.hpp"

namespace MVS
{
  /**
  * @brief Constructor
  * @param pos Initial position
  * @param nor Initial normal
  */
  PutativePoint::PutativePoint( const openMVG::Vec3 & pos , const openMVG::Vec3 & nor , const openMVG::Vec3 & col )
    : m_cur_pt_sum( pos ) ,
      m_cur_nor_sum( nor ) ,
      m_cur_col_sum( col ) ,
      m_nb_cand( 1 )
  {

  }




  void PutativePoint::SetInitialPoint( const openMVG::Vec3 & pt , const openMVG::Vec3 & nor , const openMVG::Vec3 & col )
  {
    m_cur_pt_sum = pt ;
    m_cur_nor_sum = nor ;
    m_cur_col_sum = col ;

    m_nb_cand = 1 ;
  }


  /**
  * @brief Add a new candidate for this point
  * @param pos Position of the candidate
  * @param nor Normal for this candidate
  */
  void PutativePoint::AddCandidate( const openMVG::Vec3 & pos , const openMVG::Vec3 & nor , const openMVG::Vec3 & col )
  {
    m_cur_pt_sum += pos ;
    m_cur_nor_sum += nor ;
    m_cur_col_sum += col ;

    ++m_nb_cand ;
  }

  /**
  * @brief Get final position of this point ad the mean of all candidates
  * @return position,normal for the final point
  */
  std::tuple<openMVG::Vec3, openMVG::Vec3, openMVG::Vec3> PutativePoint::GetPoint( void ) const
  {
    const openMVG::Vec3 res_pt  = m_cur_pt_sum  / ( double ) m_nb_cand ;
    const openMVG::Vec3 res_nor = m_cur_nor_sum / ( double ) m_nb_cand ;
    const openMVG::Vec3 res_col = m_cur_col_sum / ( double ) m_nb_cand ;

    return std::make_tuple( res_pt , res_nor.normalized() , res_col ) ;
  }

  /**
  * @brief Get number of candidate to generate this point
  * @return Number of candidate
  */
  unsigned long PutativePoint::NbCandidate( void ) const
  {
    return m_nb_cand ;
  }
}