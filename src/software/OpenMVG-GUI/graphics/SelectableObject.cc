#include "SelectableObject.hh"

namespace openMVG_gui
{

/**
* @brief ctr
* @param selected current selection
*/
SelectableObject::SelectableObject( const bool selected )
  : m_selected( selected )
{

}

/**
* @brief get current selection state
* @retval true if object is selected
* @retval false if object is not selected
*/
bool SelectableObject::selected( void ) const
{
  return m_selected ;
}

/**
* @brief set current selection
* @param sel new selection
*/
void SelectableObject::setSelected( const bool sel )
{
  m_selected = sel ;
}

/**
* @brief switch selection state
*/
void SelectableObject::switchSelection( void )
{
  m_selected = ! m_selected ;
}

} // namespace openMVG_gui