#include "ApplicationSettingsDialog.hh"

#include <QColorDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace openMVG_gui
{

ApplicationSettingsDialog::ApplicationSettingsDialog( QWidget *parent, const ApplicationSettings &setting )
    : QDialog( parent )
    , m_initialSettings( setting ) , 
      m_currentSettings( setting )
{
  buildInterface();
  makeConnections();

  update();
}

/**
 * @brief Get the settings in the dialog
 * @return current settings of the dialog
 */
ApplicationSettings ApplicationSettingsDialog::settings( void ) const
{
  return m_currentSettings;
}

/**
 * @brief action to be executed when user click on cancel button
 */
void ApplicationSettingsDialog::onCancel( void )
{
  done( QDialog::Rejected );
}

/**
 * @brief action to be executed when user click on OK button
 */
void ApplicationSettingsDialog::onOk( void )
{
  done( QDialog::Accepted );
}

/**
 * @brief When user click on the background color indicator
 */
void ApplicationSettingsDialog::onWantToSetBackgroundColor( void )
{
  const openMVG::Vec4 backgroundColor = m_currentSettings.viewBackgroundColor();
  const QColor        col( backgroundColor[ 0 ] * 255, backgroundColor[ 1 ] * 255, backgroundColor[ 2 ] * 255,
                    backgroundColor[ 3 ] * 255 );

  QColorDialog dlg( col );

  int res = dlg.exec();
  if ( res == QColorDialog::Accepted )
  {
    const QColor newColor = dlg.currentColor();
    m_view_background_color_indicator->setColor( newColor );

    const double r = newColor.redF();
    const double g = newColor.greenF();
    const double b = newColor.blueF();
    const double a = newColor.alphaF();

    m_currentSettings.setViewBackgroundColor( {r, g, b, a} );
  }
}

/**
 * @brief Get back to the initial settings
 */
void ApplicationSettingsDialog::reinitSettings( void )
{
  m_currentSettings = m_initialSettings;
}

/**
 * @brief Reset to the application default settings
 */
void ApplicationSettingsDialog::resetDefaultSettings( void )
{
  m_currentSettings = ApplicationSettings();
}

void ApplicationSettingsDialog::buildInterface( void )
{
  QGroupBox *  viewGrp              = new QGroupBox( "3d View" );
  QGridLayout *viewGrpLayout        = new QGridLayout;
  QLabel *     lblBackgroundColor   = new QLabel( "Background color" );
  m_view_background_color_indicator = new ColorIndicatorWidget;

  viewGrpLayout->addWidget( lblBackgroundColor, 0, 0 );
  viewGrpLayout->addWidget( m_view_background_color_indicator, 0, 1 );
  viewGrp->setLayout( viewGrpLayout );

  m_btn_cancel = new QPushButton( "Cancel" );
  m_btn_cancel->setDefault( false );
  m_btn_ok = new QPushButton( "OK" );
  m_btn_ok->setDefault( true );

  QHBoxLayout *btnLayout = new QHBoxLayout;
  btnLayout->addStretch();
  btnLayout->addWidget( m_btn_cancel );
  btnLayout->addWidget( m_btn_ok );

  QVBoxLayout *mainLayout = new QVBoxLayout;

  mainLayout->addWidget( viewGrp );
  mainLayout->addLayout( btnLayout );

  setLayout( mainLayout );
}

void ApplicationSettingsDialog::makeConnections( void )
{
  connect( m_btn_cancel, SIGNAL( clicked() ), this, SLOT( onCancel() ) );
  connect( m_btn_ok, SIGNAL( clicked() ), this, SLOT( onOk() ) );

  connect( m_view_background_color_indicator, SIGNAL( clicked() ), this, SLOT( onWantToSetBackgroundColor() ) );
}

void ApplicationSettingsDialog::update( void )
{
  // Update interface using current settings

  // 3d view background color
  {
    const openMVG::Vec4 backgroundColor = m_currentSettings.viewBackgroundColor();

    const QColor col( backgroundColor[ 0 ] * 255, backgroundColor[ 1 ] * 255, backgroundColor[ 2 ] * 255,
                      backgroundColor[ 3 ] * 255 );
    m_view_background_color_indicator->setColor( col );
  }
}

} // namespace openMVG_gui
