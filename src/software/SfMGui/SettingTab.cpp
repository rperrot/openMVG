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
  m_feature_mode_label = new QLabel( "Quality" ) ;
  m_pipeline_type_label = new QLabel( "Type" ) ;
  m_camera_model_label = new QLabel( "Model" ) ;

  m_combo_feature_type = new QComboBox( this ) ;
  m_combo_feature_setting = new QComboBox( this ) ;
  m_combo_pipeline_type = new QComboBox( this ) ;
  m_combo_camera_model = new QComboBox( this ) ;

  QGroupBox * groupCamera = new QGroupBox( "Camera" , this ) ;
  QGroupBox * groupFeatures = new QGroupBox( "Features" , this ) ;
  QGroupBox * groupPipeline = new QGroupBox( "Pipeline" , this ) ;

  QVBoxLayout * mainLayout = new QVBoxLayout ;

  QGridLayout * cameraLayout = new QGridLayout ;
  QGridLayout * featureLayout = new QGridLayout ;
  QGridLayout * pipelineLayout = new QGridLayout ;

  cameraLayout->addWidget( m_camera_model_label , 0 , 0 ) ;
  cameraLayout->addWidget( m_combo_camera_model , 0 , 1 ) ;
  groupCamera->setLayout( cameraLayout ) ;

  featureLayout->addWidget( m_feature_type_label , 0 , 0 ) ;
  featureLayout->addWidget( m_combo_feature_type , 0 , 1 ) ;
  featureLayout->addWidget( m_feature_mode_label , 1 , 0 ) ;
  featureLayout->addWidget( m_combo_feature_setting , 1 , 1 ) ;
  groupFeatures->setLayout( featureLayout ) ;

  pipelineLayout->addWidget( m_pipeline_type_label , 0 , 0 ) ;
  pipelineLayout->addWidget( m_combo_pipeline_type , 0 , 1 ) ;
  groupPipeline->setLayout( pipelineLayout ) ;

  mainLayout->addWidget( groupCamera ) ;
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
  for( auto feat_type = SettingIterator<FeatureType>::begin() ;
       feat_type != SettingIterator<FeatureType>::end() ;
       feat_type = SettingIterator<FeatureType>::next( feat_type ) )
  {
    m_combo_feature_type->addItem( ToString( feat_type ).c_str() );
  }

  // Feature mode
  for( auto feat_mode = SettingIterator<FeatureMode>::begin() ;
       feat_mode != SettingIterator<FeatureMode>::end() ;
       feat_mode = SettingIterator<FeatureMode>::next( feat_mode ) )
  {
    m_combo_feature_setting->addItem( ToString( feat_mode ).c_str() ) ;
  }

  // Pipeline type
  for( auto pipeline_type = SettingIterator<PipelineType>::begin() ;
       pipeline_type != SettingIterator<PipelineType>::end() ;
       pipeline_type = SettingIterator<PipelineType>::next( pipeline_type ) )
  {
    m_combo_pipeline_type->addItem( ToString( pipeline_type ).c_str() ) ;
  }

  // Camera model
  for( auto camera_model = SettingIterator<CameraModelType>::begin() ;
       camera_model != SettingIterator<CameraModelType>::end() ;
       camera_model = SettingIterator<CameraModelType>::next( camera_model ) )
  {
    m_combo_camera_model->addItem( ToString( camera_model ).c_str() ) ;
  }

  SfMSettings default_settings ;
  SetSettings( default_settings ) ;
}

/**
* @brief Set all settings
* @param set New settings
*/
void SettingTab::SetSettings( const SfMSettings & set )
{
  // Feature type
  const std::string feat_type = ToString( set.m_feat_type ) ;
  for( int i = 0 ; i < m_combo_feature_type->count() ; ++i )
  {
    if( m_combo_feature_type->itemText( i ).toStdString() == feat_type )
    {
      m_combo_feature_type->setCurrentIndex( i ) ;
      // activate this item
      break ;
    }
  }

  // Feature quality
  const std::string feat_mode = ToString( set.m_feat_mode ) ;
  for( int i = 0 ; i < m_combo_feature_setting->count() ; ++i )
  {
    if( m_combo_feature_setting->itemText( i ).toStdString() == feat_mode )
    {
      // Activate this item
      m_combo_feature_setting->setCurrentIndex( i ) ;
      break ;
    }
  }

  // Pipeline type
  const std::string pipeline_type = ToString( set.m_pipeline_type ) ;
  for( int i = 0 ; i < m_combo_pipeline_type->count() ; ++i )
  {
    if( m_combo_pipeline_type->itemText( i ).toStdString() == pipeline_type )
    {
      // Activate this item
      m_combo_pipeline_type->setCurrentIndex( i ) ;
      break;
    }
  }

  // Camera model type
  const std::string camera_model = ToString( set.m_camera_model ) ;
  for( int i = 0 ; i < m_combo_camera_model->count() ; ++i )
  {
    if( m_combo_camera_model->itemText( i ).toStdString() == camera_model )
    {
      m_combo_camera_model->setCurrentIndex( i ) ;
      break ;
    }
  }

}

/**
* @brief Get all settings
* @return Get settings
*/
SfMSettings SettingTab::GetSettings( void ) const
{
  const std::string feat_type = m_combo_feature_type->currentText().toStdString() ;
  const std::string feat_mode = m_combo_feature_setting->currentText().toStdString() ;
  const std::string pipeline_type = m_combo_pipeline_type->currentText().toStdString() ;
  const std::string camera_model = m_combo_camera_model->currentText().toStdString() ;

  SfMSettings res ;

  FromString( feat_type , res.m_feat_type ) ;
  FromString( feat_mode , res.m_feat_mode ) ;
  FromString( pipeline_type , res.m_pipeline_type ) ;
  FromString( camera_model , res.m_camera_model ) ;

  return res ;
}

/**
* @brief Reset setting to default value
*/
void SettingTab::Reset( void )
{
  SfMSettings sett ;
  SetSettings( sett ) ;
}


} // namespace SfMGui
} // namespace openMVG