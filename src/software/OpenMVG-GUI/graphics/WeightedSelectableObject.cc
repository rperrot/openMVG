#include "WeightedSelectableObject.hh"

namespace openMVG_gui
{
/**
 * @brief Ctr
 * @param w Weight of the selection
 * @note Default selection mode is unselected
 */
WeightedSelectableObject::WeightedSelectableObject( const bool selected, const double w ) :
    SelectableObject( selected ), m_weight( w )
{
}

/**
 * @brief Weight of the selection
 */
double WeightedSelectableObject::selectionWeight( void ) const
{
  return m_weight;
}

/**
 * @brief Set weight of the selection
 * @param w Weight of the selection
 */
void WeightedSelectableObject::setSelectionWeight( const double w )
{
  m_weight = w;
}

} // namespace openMVG_gui
