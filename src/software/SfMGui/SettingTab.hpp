#ifndef _OPENMVG_SFMGUI_SETTING_TAB_HPP_
#define _OPENMVG_SFMGUI_SETTING_TAB_HPP_

#include <QWidget>
#include <QComboBox>
#include <QLabel>

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Tab used to manage all settings of the SfM process
*/
class SettingTab : public QWidget
{
  public:

    /**
    * @brief Ctr
    */
    SettingTab( QWidget * parent = nullptr ) ;

  private:

    /**
    * @brief Build interface
    */
    void BuildInterface( void ) ;

    /**
    * @brief Populate all settings
    */
    void FillSettings( void ) ;

    QLabel * m_feature_type_label ;
    QLabel * m_feature_mode_label ;
    QLabel * m_pipeline_type_label ;

    QComboBox * m_combo_feature_type ;
    QComboBox * m_combo_feature_setting ;
    QComboBox * m_combo_pipeline_type ;
} ;

} // namespace SfMGui
} // namespace openMVG

#endif