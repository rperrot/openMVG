#include "MatchingCost.hpp"
#include "Util.hpp"

#include <algorithm>
#include <cmath>

namespace MVS
{
  /**
   * @brief compute matching cost between two pixels p and q
   * @param Ip Intensity at pixel p
   * @param Gp Gradient at pixel p
   * @param Iq Intensity at pixel q
   * @param Gq Gradient at pixel q
   * @param alpha balance between gradient and intensity (from 0 -> all intensity to 1 -> all gradient)
   * @param TauImg Threshold on intensity
   * @param TauGrad Threshold on gradient
   * @return cost value between pixel p and q
   * @note this is equation (2) in ref paper
   * @ref Massively Parallel Multiview Stereopsis by Surface Normal Diffusion ; Galliani , Lasinger , Schindler
   */
  double ComputeMatchingCost( const unsigned char Ip , const double Gp ,
                              const unsigned char Iq , const double Gq ,
                              const double alpha ,
                              const double TauImg ,
                              const double TauGrad )
  {
    const double diff_intens = static_cast<double>( Ip ) - static_cast<double>( Iq ) ;
    const double diff_grad   = ( Gp - Gq ) ;

    const double norm_intens = std::fabs( diff_intens ) ;
    const double norm_grad   = std::fabs( diff_grad ) ;

    const double cost_intens = std::min( norm_intens , TauImg ) ;
    const double cost_grad   = std::min( norm_grad , TauGrad ) ;

    return ( 1.0 - alpha ) * cost_intens + alpha * cost_grad ;
  }

  /**
  * @brief compute matching cost between two pixels p and q
  * @param Ip Intensity at pixel p
  * @param Gp Gradient (Gx,Gy,Gxy,Gyx) at pixel p
  * @param Iq Intensity at pixel q
  * @param Gq Gradient (Gx,Gy,Gxy,Gyx) at pixel q
  * @param alpha balance between gradient and intensity (from 0 -> all intensity to 1 -> all gradient)
  * @param TauImg Threshold on intensity
  * @param TauGrad Threshold on gradient
  * @return cost value between pixel p and q
  * @note this is equation (2) in ref paper
  * @ref Massively Parallel Multiview Stereopsis by Surface Normal Diffusion ; Galliani , Lasinger , Schindler
  */
  double ComputeMatchingCost( const unsigned char Ip , const openMVG::Vec4 & Gp ,
                              const unsigned char Iq , const openMVG::Vec4 & Gq ,
                              const double alpha ,
                              const double TauImg ,
                              const double TauGrad )
  {
    const double diff_intens = static_cast<double>( Ip ) - static_cast<double>( Iq ) ;
    //   const openMVG::Vec4 diff_grad   = (Gp - Gq) ;

    const double norm_intens = std::fabs( diff_intens ) ;
    const double norm_grad = ( Gp - Gq ).cwiseAbs().sum() ;
    //    const double norm_grad   = std::fabs( diff_grad[0] ) + std::fabs( diff_grad[1] ) ;

    const double cost_intens = std::min( norm_intens , TauImg ) ;
    const double cost_grad   = std::min( norm_grad , TauGrad ) ;

    return ( 1.0 - alpha ) * cost_intens + alpha * cost_grad ;
  }


  /**
  * @brief compute matching cost between two pixels p and q
  * @param Ip Intensity at pixel p
  * @param Gp Gradient at pixel p
  * @param Iq Intensity at pixel q
  * @param Gq Gradient at pixel q
  * @param alpha balance between gradient and intensity (from 0 -> all intensity to 1 -> all gradient)
  * @param TauImg Threshold on intensity
  * @param TauGrad Threshold on gradient
  * @return cost value between pixel p and q
  * @note this is equation (2) in ref paper
  * @ref Massively Parallel Multiview Stereopsis by Surface Normal Diffusion ; Galliani , Lasinger , Schindler
  */
  double ComputeMatchingCost( const unsigned char Ip , const double Gpx , const double Gpy ,
                              const unsigned char Iq , const double Gqx , const double Gqy ,
                              const double alpha ,
                              const double TauImg ,
                              const double TauGrad )
  {
    const double diff_intens = l1_norm( static_cast<double>( Ip ) - static_cast<double>( Iq ) ) ;
    const double cost_intens = std::min( TauImg , diff_intens ) ;

    const double gx = ( Gpx - Gqx ) ;
    const double gy = ( Gpy - Gqy ) ;

    const double diff_grad = ( l1_norm( gx ) + l1_norm( gy ) ) ;
    const double cost_grad = std::min( diff_grad , TauGrad ) ;

    return ( 1.0 - alpha ) * cost_intens + alpha * cost_grad ;
  }

}