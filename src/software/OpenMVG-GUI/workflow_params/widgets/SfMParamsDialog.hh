#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_SFM_PARAMS_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_WORKFLOW_PARAMS_WIDGETS_SFM_PARAMS_DIALOG_HH_

#include "Project.hh"
#include "workflow_params/GlobalSfMParams.hh"
#include "workflow_params/IncrementalSfMParams.hh"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>


namespace openMVG_gui
{

/**
* @brief class holding SfM params
*/
class SfMParamsDialog : public QDialog
{
  public:

    /**
    * @brief Ctr
    * @param method Initial SfM method
    * @param incr_params Initial sequential SfM params
    * @param glo_params Initial global SfM params
    */
    SfMParamsDialog( QWidget * parent ,
                     const SfMMethod method ,
                     const IncrementalSfMParams & incr_params ,
                     const GlobalSfMParams & glo_params ,
                     const std::vector<int> & imageIds ) ;


    /**
    * @brief Current SfM method
    * @return Get current SfM method
    */
    SfMMethod method( void ) const ;

    /**
    * @brief Current incremental params
    * @return Get current incremental SfM params
    */
    IncrementalSfMParams incrementalParams( void ) const ;

    /**
    * @brief Current global params
    * @return Get current global SfM params
    */
    GlobalSfMParams globalParams( void ) const ;

  public slots :


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

  private slots :

    /**
    * @brief Action to be executed when user has selected a SfM method
    */
    void onSelectMethod( void ) ;

    /**
    * @brief Action to be executed when user has checked/unchecked automatic pair for incremental SfM
    */
    void onCheckAuto( void ) ;

  private:

    /**
    * @brief Set params
    * @param meth SfM Method
    * @param incr_params Incremental params
    * @param glo_params Global params
    */
    void setParams( const SfMMethod meth ,
                    const IncrementalSfMParams & incr_params ,
                    const GlobalSfMParams & glo_params ) ;

    void buildInterface( void ) ;
    void makeConnections( void ) ;

    // Method
    QRadioButton * m_radio_incr ;
    QRadioButton * m_radio_glo ;

    // Incremental
    QGroupBox * m_grp_incremental ;
    QComboBox * m_incr_intrinsic_type ;
    QComboBox * m_incr_refine_type ;
    QCheckBox * m_incr_use_automatic_pair ;
    QComboBox * m_incr_first_image_id ;
    QComboBox * m_incr_second_image_id ;

    // Global
    QGroupBox * m_grp_global ;
    QComboBox * m_glo_translation_avg_method ;
    QComboBox * m_glo_rotation_avg_method ;
    QComboBox * m_glo_refine_type ;

    QPushButton * m_ok_btn ;
    QPushButton * m_cancel_btn ;
    QPushButton * m_reset_btn ;

    SfMMethod m_initial_method ;
    IncrementalSfMParams m_initial_seq_params ;
    GlobalSfMParams m_initial_glo_params ;

    // The list of ids of images to provide in initial pair selection
    std::vector< int > m_image_ids ;

    Q_OBJECT
} ;

} // namespace openMVG_gui


#endif