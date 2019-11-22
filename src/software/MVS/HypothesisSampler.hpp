#ifndef OPENMVG_MVS_HYPOTHESIS_SAMPLER_HPP
#define OPENMVG_MVS_HYPOTHESIS_SAMPLER_HPP

#include "DepthMapComputationParameters.hpp"

#include <openMVG/image/image_container.hpp>
#include <openMVG/numeric/numeric.h>

#include <vector>

namespace MVS
{

/**
 * @brief Get the set of hyptohesis planes based on the current parameters.
 *  Theses are hypothesis used for propagation. 
 * 
 * @param planes      The set of planes 
 * @param cost        The current costs
 * @param id_row      Y-Coord pixel 
 * @param id_col      X-Coord pixel 
 * @param params      The current parameters used to sample the hypothesis
 * 
 * @return The set of hypothesis and the real coordinates (x,y) of each hypothesis
 */
std::pair<std::vector<openMVG::Vec4>, std::vector<openMVG::Vec2i>> getPropagationHypothesis( const openMVG::image::Image<openMVG::Vec4>& planes,
                                                                                             const openMVG::image::Image<double>&        cost,
                                                                                             const int                                   id_row,
                                                                                             const int                                   id_col,
                                                                                             const DepthMapComputationParameters&        params );

/**
 * @brief 
 * 
 */
std::vector<openMVG::Vec4> getRefinementHypothesis( const openMVG::image::Image<openMVG::Vec4>& planes,
                                                    const int                                   id_row,
                                                    const int                                   id_col,
                                                    const DepthMapComputationParameters&        params );

} // namespace MVS

#endif