#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_MATCHING_PARAMS_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_MATCHING_PARAMS_DIALOG_HH_

#include "workflow_params/MatchingParams.hh"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QPushButton>
#include <QSpinBox>


namespace openMVG_gui
{

/**
* @brief Interface used to select matching/geometric filtering settings
*/
class MatchingParamsDialog : public QDialog
{
  public:

    /**
    * @brief Ctr
    * @param parent Parent Widget
    * @param initParams Initial Parameters passed to the dialog
    */
    MatchingParamsDialog( QWidget * parent , const MatchingParams & initParams );

    /**
    * @brief Get current parameters
    * @return Current parameters corresponding to the interface
    */
    MatchingParams params( void ) ;

  public slots:

    /**
    * @brief action to be executed when user click on cancel button
    */
    void onCancel( void ) ;

    /**
    * @brief action to be executed when user click on ok button
    */
    void onOk( void ) ;

    /**
    * @brief Action to be executed when user click on reset button
    */
    void onReset( void ) ;

  private:

    /**
    * @brief Set params to a specified value
    * @param value New value
    */
    void setParams( const MatchingParams & value ) ;

    void buildInterface( void ) ;
    void makeConnections( void ) ;

    QComboBox * m_params_matching_method ;
    QDoubleSpinBox * m_ratio ;
    QComboBox * m_params_geometry_type ;
    QSpinBox * m_max_iteration ;

    QPushButton * m_ok_btn ;
    QPushButton * m_cancel_btn ;
    QPushButton * m_reset_btn ;

    MatchingParams m_initial_params ;

    Q_OBJECT
};

} // namespace openMVG_gui

#endif