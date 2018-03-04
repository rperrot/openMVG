#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_SFM_HELPER_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_UTILS_SFM_HELPER_HH_

#include <tuple>
#include <utility>
#include <vector>

// fwrd declaration
namespace openMVG
{
namespace sfm
{
struct SfM_Data ;
}
}

namespace openMVG_gui
{

/**
 * @brief Helper class used to compute various infos/stats/metrics on SfM_Data
 */
class SfMDataHelper
{
  public:

    /**
     * @brief Ctr
     * @param sfm_data the structure on which information will be extracted
     */
    SfMDataHelper( std::shared_ptr<openMVG::sfm::SfM_Data> sfm_data = nullptr ) ;

    /**
     * @brief Get all views id that are linked to a given one
     * @return vector of all linked views
     */
    std::vector< int > linkedViews( const int id ) const ;

    /**
     * @brief Get all views id that are linked to a given one with their corresponding strength
     * @return vector of all linked views with their corresponding strength
     * @note Strength is normalized between 0 and 1
     * @note Strength = 1 is the maximum (ie: best link with the given view wrt the input id)
     */
    std::vector< std::pair<int, double> > linkedViewsWithStrength( const int id ) const ;

    /**
     * @brief Get all view pairs in the scene
     * @return vector of all view pairs
     */
    std::vector< std::pair<int, int> > allViewPairs( void ) const ;

    /**
     * @brief Get all view pairs in the scene with their corresponding strength
     * @return vector of all view pairs with their corresponding strength
     * @note Strength is normalized between 0 and 1
     * @note Strength = 1 is the maximum (ie: best link among all pairs)
     */
    std::vector< std::tuple< int , int , double > > allViewPairsWithStrength( void ) const ;

  private:

    /// The sfm data
    std::shared_ptr<openMVG::sfm::SfM_Data> m_sfm_data ;
} ;

} // namespace openMVG_gui

#endif