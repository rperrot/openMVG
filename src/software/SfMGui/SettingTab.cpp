#include "software/SfMGui/SettingTab.hpp"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>

namespace openMVG
{
namespace SfMGui
{

/**
* @brief Ctr
*/
SettingTab::SettingTab( QWidget * parent )
  : QWidget( parent )
{
  BuildInterface() ;
  FillSettings() ;
}

/**
* @brief Build interface
*/
void SettingTab::BuildInterface( void )
{
  m_feature_type_label = new QLabel( "Type" ) ;
  m_feature_mode_label = new QLabel( "Mode" ) ;
  m_pipeline_type_label = new QLabel( "Type" ) ;

  m_combo_feature_type = new QComboBox( this ) ;
  m_combo_feature_setting = new QComboBox( this ) ;
  m_combo_pipeline_type = new QComboBox( this ) ;

  QGroupBox * groupFeatures = new QGroupBox( "Features" , this ) ;
  QGroupBox * groupPipeline = new QGroupBox( "Pipeline" , this ) ;

  QVBoxLayout * mainLayout = new QVBoxLayout ;

  QGridLayout * featureLayout = new QGridLayout ;
  QGridLayout * pipelineLayout = new QGridLayout ;

  featureLayout->addWidget( m_feature_type_label , 0 , 0 ) ;
  featureLayout->addWidget( m_combo_feature_type , 0 , 1 ) ;
  featureLayout->addWidget( m_feature_mode_label , 1 , 0 ) ;
  featureLayout->addWidget( m_combo_feature_setting , 1 , 1 ) ;
  groupFeatures->setLayout( featureLayout ) ;

  pipelineLayout->addWidget( m_pipeline_type_label , 0 , 0 ) ;
  pipelineLayout->addWidget( m_combo_pipeline_type , 0 , 1 ) ;
  groupPipeline->setLayout( pipelineLayout ) ;

  mainLayout->addWidget( groupFeatures ) ;
  mainLayout->addWidget( groupPipeline ) ;
  mainLayout->addStretch( ) ;

  setLayout( mainLayout ) ;
}

/**
* @brief Populate all settings
*/
void SettingTab::FillSettings( void )
{
  // Features types
  m_combo_feature_type->addItem( "SIFT" ) ;
  m_combo_feature_type->addItem( "AKAZE" ) ;

  // Features mode
  m_combo_feature_setting->addItem( "LOW" ) ;
  m_combo_feature_setting->addItem( "HIGH" ) ;
  m_combo_feature_setting->addItem( "ULTRA" ) ;

  // Pipeline type
  m_combo_pipeline_type->addItem( "Incremental" ) ;
  m_combo_pipeline_type->addItem( "Global" ) ;
}


} // namespace SfMGui
} // namespace openMVG