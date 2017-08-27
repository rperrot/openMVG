#include "ClusterComputationSettingsDialog.hh"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace openMVG_gui
{

/**
* @brief Ctr
* @param lower_bound Lower bound on clustering
* @param upper_bound Upper bound on clustering
* @param grid_size Voxel grid size
*/
ClusterComputationSettingsDialog::ClusterComputationSettingsDialog(  QWidget * parent ,
    const int lower_bound ,
    const int upper_bound ,
    const float grid_size )
  : QDialog( parent )
{
  buildInterface() ;
  makeConnections() ;

  m_spin_lower_bound->setValue( lower_bound ) ;
  m_spin_upper_bound->setValue( upper_bound ) ;
  m_spin_voxel_grid->setValue( static_cast<double>( grid_size ) ) ;
}

/**
* @brief Get lower bound
* @return lower bound
*/
int ClusterComputationSettingsDialog::clusterLowerBound( void ) const
{
  return m_spin_lower_bound->value() ;
}

/**
* @brief Get upper bound
* @return upper bound
*/
int ClusterComputationSettingsDialog::clusterUpperBound( void ) const
{
  return m_spin_upper_bound->value() ;
}

/**
* @brief Get voxel grid size
* @return voxel grid size
*/
float ClusterComputationSettingsDialog::clusterGridSize( void ) const
{
  return static_cast<float>( m_spin_voxel_grid->value() ) ;
}


/**
* @brief action to be executed when user click on cancel button
*/
void ClusterComputationSettingsDialog::onCancel( void )
{
  done( QDialog::Rejected ) ;
}

/**
* @brief action to be executed when user click on OK button
*/
void ClusterComputationSettingsDialog::onOk( void )
{
  done( QDialog::Accepted ) ;
}

/**
* @brief Build interface
*/
void ClusterComputationSettingsDialog::buildInterface( void )
{
  m_lbl_lower_bound = new QLabel( "Lower bound" ) ;
  m_lbl_upper_bound = new QLabel( "Upper bound" ) ;
  m_lbl_grid_size   = new QLabel( "Voxel grid size" ) ;

  m_spin_lower_bound = new QSpinBox ;
  m_spin_lower_bound->setRange( 1 , 100000 ) ;
  m_spin_upper_bound = new QSpinBox ;
  m_spin_upper_bound->setRange( 1 , 100000 ) ;
  m_spin_voxel_grid = new QDoubleSpinBox ;
  m_spin_voxel_grid->setRange( 0.001 , 10000.0 ) ;

  m_btn_cancel = new QPushButton( "Cancel" ) ;
  m_btn_cancel->setDefault( false ) ;
  m_btn_ok = new QPushButton( "OK" ) ;
  m_btn_ok->setDefault( true ) ;

  QHBoxLayout * btnLayout = new QHBoxLayout ;
  btnLayout->addStretch() ;
  btnLayout->addWidget( m_btn_cancel ) ;
  btnLayout->addWidget( m_btn_ok ) ;

  QGridLayout * paramLayout = new QGridLayout ;
  paramLayout->addWidget( m_lbl_lower_bound , 0 , 0 ) ;
  paramLayout->addWidget( m_spin_lower_bound , 0 , 1 ) ;
  paramLayout->addWidget( m_lbl_upper_bound , 1 , 0 ) ;
  paramLayout->addWidget( m_spin_upper_bound , 1 , 1 ) ;
  paramLayout->addWidget( m_lbl_grid_size , 2 , 0 ) ;
  paramLayout->addWidget( m_spin_voxel_grid , 2 , 1 ) ;

  QVBoxLayout * mainLayout = new QVBoxLayout ;
  mainLayout->addLayout( paramLayout ) ;
  mainLayout->addLayout( btnLayout ) ;

  setLayout( mainLayout ) ;
}

/**
* @brief make connections between elements
*/
void ClusterComputationSettingsDialog::makeConnections( void )
{
  connect( m_btn_cancel , SIGNAL( clicked() ) , this , SLOT( onCancel() ) ) ;
  connect( m_btn_ok , SIGNAL( clicked() ) , this , SLOT( onOk() ) );
}

} // namespace openMVG_gui