#ifndef _OPENMVG_SFMGUI_RESULT_TAB_HPP_
#define _OPENMVG_SFMGUI_RESULT_TAB_HPP_

#include <QWidget>

namespace openMVG
{
namespace SfMGui
{

/**
* Tab used to show results of the SfM process (openGL viewer)
*/
class ResultTab : public QWidget
{
  public:

    /**
    * @brief Ctr
    * @param parent Parent
    */
    ResultTab( QWidget * parent = nullptr ) ;

    /**
    * @brief Reset view
    */
    void Reset( void ) ;

  private:

    /**
    * @brief Build interface
    */
    void BuildInterface( void ) ;
} ;
} // namespace SfMGui
} // namespace openMVG

#endif