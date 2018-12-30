#ifndef _OPENMVG_MVS_JOINT_VIEW_SELECTION_HPP_
#define _OPENMVG_MVS_JOINT_VIEW_SELECTION_HPP_

#include "openMVG/numeric/numeric.h"

#include <algorithm>
#include <vector>

namespace MVS
{

/**
 * @brief Compute for a matrix of cost, a selection set of good views
 * 
 * @param selection_set   The selection for each view (1 if selected, 0 if rejected)
 * @param cost_m          The cost matrix (1 col per view)
 * @param threshold       The lower threshold 
 * @param tau_up          The higher threshold 
 * @param n1              Minimum number of value lower than the low_threshold to be considered as a good view
 * @param n2              Maximum number of value higher than the high threshold to be considered as a good view 
 * 
 * @note A view is considered as a good view if :
 * - it as at least n1 values with cost under than low threshold  
 * - it as at most n2 values with cost higher than high threshold 
 */
static inline void ComputeSelectionSet( std::vector<int>&   selection_set,
                                        const openMVG::Mat& cost_m,
                                        const double        threshold,
                                        const double        tau_up,
                                        const int           n1,
                                        const int           n2 )
{
  const int nb_view = cost_m.cols();
  selection_set.resize( nb_view );
  for ( int id_view = 0; id_view < nb_view; ++id_view )
  {
    // Compute number of cost that are less than threshold
    // Compute number of cost that are more than tup
    int nb_low  = 0;
    int nb_high = 0;
    for ( int id_hyp = 0; id_hyp < cost_m.rows(); ++id_hyp )
    {
      const double cur_cost = cost_m( id_hyp, id_view );
      if ( cur_cost < threshold )
      {
        ++nb_low;
      }
      if ( cur_cost > tau_up )
      {
        ++nb_high;
      }
    }
    // Test if view is selected
    selection_set[ id_view ] = ( nb_low > n1 && nb_high < n2 ) ? 1 : 0;
  }
}

/**
 * @brief  Compute importance value per view 
 * 
 * @param importance      The importance value (1 value per view)
 * @param selection_set   The selection set (1 if view is selected, 0 if rejected)
 * @param cost_m          The cost matrix 
 * @param beta            Gaussian scale 
 * 
 * Importance is mean of confidence value for each selected view (it is 0 for rejected view) 
 */
static inline void ComputeViewImportance( std::vector<double>&    importance,
                                          const std::vector<int>& selection_set,
                                          const openMVG::Mat&     cost_m,
                                          const double            beta )
{
  const int nb_view = cost_m.cols();
  const int nb_hyp  = cost_m.rows();
  importance.resize( nb_view );
  for ( int id_view = 0; id_view < nb_view; ++id_view )
  {
    importance[ id_view ] = 0.0;
    if ( selection_set[ id_view ] == 0 )
      continue;

    for ( int id_hyp = 0; id_hyp < nb_hyp; ++id_hyp )
    {
      const double c = cost_m( id_hyp, id_view );
      importance[ id_view ] += std::exp( -( c * c ) / ( 2.0 * beta * beta ) );
    }
    importance[ id_view ] /= static_cast<double>( nb_hyp );
  }
}

/**
 * @brief Only keep the best N importance values 
 * 
 * @param importance The importance values 
 * @param k          The maximum number of view to keep 
 */
static inline void FilterBestNImportance( std::vector<double>& importance, const int k )
{
  // Only keep the K most important views
  std::vector<double> tmp = importance;
  std::sort( tmp.begin(), tmp.end() );
  const int    index = std::min( importance.size(), (size_t)k );
  const double th    = tmp[ tmp.size() - index ];
  for ( auto& imp : importance )
  {
    if ( imp < th )
    {
      imp = 0.0;
    }
  }
}

/**
 * @brief Compute per hypothesis cost 
 * 
 * @param cost            Per hypothesis cost 
 * @param view_importance Importance value per view 
 * @param cost_m          Cost matrix 
 */
static inline void ComputePerHypothesisCost( std::vector<double>&                 cost,
                                             const std::vector<double>&           view_importance,
                                             const openMVG::Mat&                  cost_m,
                                             const DepthMapComputationParameters& params )
{
  const double MAX_COST = DepthMapComputationParameters::metricMaxCostValue( params.metric() );

  // Equation (7) of AMHMVS paper
  const int nb_hyp  = cost_m.rows();
  const int nb_view = cost_m.cols();
  cost.resize( nb_hyp );
  for ( int id_hyp = 0; id_hyp < nb_hyp; ++id_hyp )
  {
    double sum_w = 0.0;
    double sum   = 0.0;
    for ( int id_view = 0; id_view < nb_view; ++id_view )
    {
      const double cur_imp = view_importance[ id_view ];

      sum += cur_imp * cost_m( id_hyp, id_view );
      sum_w += cur_imp;
    }
    cost[ id_hyp ] = sum_w > 0.001 ? ( sum / sum_w ) : MAX_COST;
  }
}

/**
 * @brief Determine the view with best importance 
 * 
 * @param importance  The importance values 
 * @return int        The best view index  
 */
static inline int ComputeBestView( const std::vector<double>& importance )
{
  int    id_x          = -1;
  double best_view_imp = -std::numeric_limits<double>::max();
  for ( int id_view = 0; id_view < importance.size(); ++id_view )
  {
    if ( best_view_imp < importance[ id_view ] )
    {
      best_view_imp = importance[ id_view ];
      id_x          = id_view;
    }
  }
  return id_x;
}

/**
 * @brief Update importance values depending on the last previous best view 
 * 
 * @param importance      Importance values 
 * @param selection_set   The current selection set
 * @param old_best_view   The previous best view index 
 */
static inline void UpdateImportanceBasedOnPreviousBestView( std::vector<double>&    importance,
                                                            const std::vector<int>& selection_set,
                                                            const int               old_best_view )
{
  for ( int id_view = 0; id_view < importance.size(); ++id_view )
  {
    if ( selection_set[ id_view ] )
    {
      // Give more importance to a view that were the best one and that is still in the selection set
      const double scale = ( ( old_best_view == id_view ) ? 2.0 : 1.0 );
      importance[ id_view ] *= scale;
    }
    else
    {
      // View is not in the selection set but was in the previous selection set so give it a little importance
      importance[ id_view ] = ( old_best_view == id_view ) ? 0.2 : 0.0;
    }
  }
}

} // namespace MVS

#endif