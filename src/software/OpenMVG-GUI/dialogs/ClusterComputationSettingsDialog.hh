#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_CLUSTER_COMPUTATION_SETTINGS_DIALOG_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_DIALOGS_CLUSTER_COMPUTATION_SETTINGS_DIALOG_HH_

#include <QDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

namespace openMVG_gui
{

/**
* @brief Dialog used to provide user a way to set settings of clustering
*/
class ClusterComputationSettingsDialog : public QDialog
{
  public:

    /**
    * @brief Ctr
    * @param lower_bound Lower bound on clustering
    * @param upper_bound Upper bound on clustering
    * @param grid_size Voxel grid size
    */
    ClusterComputationSettingsDialog( QWidget * parent ,
                                      const int lower_bound = 10 ,
                                      const int upper_bound = 20 ,
                                      const float grid_size = 10.f ) ;

    /**
    * @brief Get lower bound
    * @return lower bound
    */
    int clusterLowerBound( void ) const ;

    /**
    * @brief Get upper bound
    * @return upper bound
    */
    int clusterUpperBound( void ) const ;

    /**
    * @brief Get voxel grid size
    * @return voxel grid size
    */
    float clusterGridSize( void ) const ;

  public slots :

    /**
    * @brief action to be executed when user click on cancel button
    */
    void onCancel( void ) ;

    /**
    * @brief action to be executed when user click on OK button
    */
    void onOk( void ) ;


  private:

    /**
    * @brief Build interface
    */
    void buildInterface( void ) ;

    /**
    * @brief make connections between elements
    */
    void makeConnections( void ) ;


    QLabel * m_lbl_lower_bound ;
    QLabel * m_lbl_upper_bound ;
    QLabel * m_lbl_grid_size ;

    QSpinBox * m_spin_lower_bound ;
    QSpinBox * m_spin_upper_bound ;
    QDoubleSpinBox * m_spin_voxel_grid ;

    QPushButton * m_btn_ok ;
    QPushButton * m_btn_cancel ;

    Q_OBJECT
} ;

} // namespace openMVG_gui

#endif