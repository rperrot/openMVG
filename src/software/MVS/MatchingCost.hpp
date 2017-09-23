#ifndef _OPENMVG_MVS_MATCHING_COST_HPP_
#define _OPENMVG_MVS_MATCHING_COST_HPP_

#include "openMVG/numeric/numeric.h"

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
                            const double alpha = 0.9 ,
                            const double TauImg = 10.0 ,
                            const double TauGrad = 2.0 ) ;


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
                            const double alpha = 0.9 ,
                            const double TauImg = 10.0 ,
                            const double TauGrad = 2.0 ) ;

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
                            const double alpha = 0.9 ,
                            const double TauImg = 10.0 ,
                            const double TauGrad = 2.0 ) ;
} // namespace MVS

#endif