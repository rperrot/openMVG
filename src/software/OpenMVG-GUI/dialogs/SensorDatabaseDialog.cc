#include "SensorDatabaseDialog.hh"

#include "AddSensorEntryDialog.hh"
#include "SensorDatabaseModel.hh"

#include "utils/DownloadFile.hh"

#include "ApplicationSettings.hh"

#include <QDebug>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace openMVG_gui
{
SensorDatabaseDialog::SensorDatabaseDialog( QWidget *parent ) :
    QDialog( parent )
{
  buildInterface();
  makeConnections();

  resize( 800, 600 );
  setWindowTitle( "Sensor database" );
  update();
}

void SensorDatabaseDialog::setMainDatabaseFromFile( const std::string &path )
{
  SensorDatabaseModel *model = new SensorDatabaseModel( path );
  if ( m_mainTableView->model() )
  {
    m_mainTableView->model()->deleteLater();
  }
  m_mainTableView->setModel( model );
  m_mainTableView->resizeColumnsToContents();
}

/**
   * @brief Set the User Defined Database From a file 
   * 
   * @param path Path of the user database file 
   */
void SensorDatabaseDialog::setUserDefinedDatabaseFromFile( const std::string &path )
{
  SensorDatabaseModel *model = new SensorDatabaseModel( path );
  if ( m_userTableView->model() )
  {
    m_userTableView->model()->deleteLater();
  }
  m_userTableView->setModel( model );
  connect( m_userTableView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ), this, SLOT( onUserDefinedSelectionChanged() ) );
  m_userTableView->resizeColumnsToContents();

  if ( model->rowCount() > 0 )
  {
    m_removeAllSensorsBtn->setEnabled( true );
  }
}

/**
 * @brief action to be executed when user click on OK button
 */
void SensorDatabaseDialog::onOk( void )
{
  done( QDialog::Accepted );
}

/**
 * @brief When user click on the internet update btn
 *
 */
void SensorDatabaseDialog::onWantToUpdateFromInternet( void )
{
  const std::string sensor_url = "https://openmvg.org/data/sensor_width_camera_database.txt";
  const std::string out_path   = ApplicationSettings::applicationWideSensorWidthDatabasePath();

  const bool ok = DownloadFile( sensor_url, out_path );

  if ( ok )
  {
    setMainDatabaseFromFile( out_path );
  }
  else
  {
    QMessageBox::critical( this, "Error during download", "The sensor database file could not be downloaded" );
  }
}

/**
 * @brief When user want to add a new sensor to the database
 *
 */
void SensorDatabaseDialog::onWantToAddSensor( void )
{
  AddSensorEntryDialog dlg( this );
  const int            res = dlg.exec();
  if ( res == QDialog::Accepted )
  {
    SensorDatabaseModel *model;
    if ( m_userTableView->model() )
    {
      model = static_cast<SensorDatabaseModel *>( m_userTableView->model() );
    }
    else
    {
      model = new SensorDatabaseModel;
      m_userTableView->setModel( model );
    }
    model->append( dlg.brand(), dlg.model(), dlg.sensorWidth() );
    m_userTableView->resizeColumnsToContents();
    model->save( ApplicationSettings::applicationWideUserDefinedSensorWidthDatabasePath() );

    m_removeAllSensorsBtn->setEnabled( true );
  }
}

/**
  * @brief When user want to remove a sensor 
  * 
  */
void SensorDatabaseDialog::onWantToRemoveSensor( void )
{
  const QItemSelection      selection = m_userTableView->selectionModel()->selection();
  const QItemSelectionRange cur       = selection[ 0 ];

  QMessageBox::StandardButton btn = QMessageBox::question( this, "Remove selection ?", "Changes are definitive, do you really want to remove selected item from the user database ?", QMessageBox::Yes | QMessageBox::No );
  if ( btn == QMessageBox::Yes )
  {
    SensorDatabaseModel *model = static_cast<SensorDatabaseModel *>( m_userTableView->model() );
    model->removeRow( cur.top() );
    model->save( ApplicationSettings::applicationWideUserDefinedSensorWidthDatabasePath() );

    if ( model->rowCount() == 0 )
    {
      m_removeAllSensorsBtn->setEnabled( false );
    }
  }
}

/**
  * @brief When user want to clear all sensors 
  * 
  */
void SensorDatabaseDialog::onWantToClearSensors( void )
{
  QMessageBox::StandardButton btn = QMessageBox::question( this, "Clear all sensors ?", "Changes are definitive, do you really want to clear all the user database ?", QMessageBox::Yes | QMessageBox::No );
  if ( btn == QMessageBox::Yes )
  {
    std::ofstream file( ApplicationSettings::applicationWideUserDefinedSensorWidthDatabasePath(), std::ofstream::trunc );
    m_userTableView->setModel( new SensorDatabaseModel() );

    m_removeAllSensorsBtn->setEnabled( false );
  }
}

/**
   * @brief When user click on the user defined db
   * 
   */
void SensorDatabaseDialog::onUserDefinedSelectionChanged( void )
{
  if ( m_userTableView->selectionModel()->hasSelection() )
  {
    m_removeSensorBtn->setEnabled( true );
  }
  else
  {
    m_removeSensorBtn->setEnabled( false );
  }
}

void SensorDatabaseDialog::buildInterface( void )
{
  // Main database and btns
  QGroupBox *grpMainTable = new QGroupBox( "Official database" );
  m_mainTableView         = new QTableView;
  m_mainTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  QVBoxLayout *mainDatabaseLayout = new QVBoxLayout;
  mainDatabaseLayout->setSpacing( 5 );
  mainDatabaseLayout->setContentsMargins( 5, 5, 5, 5 );

  m_updateDatabaseBtn = new QPushButton( "Update database (online)" );
  m_updateDatabaseBtn->setDefault( false );
  QHBoxLayout *mainDatabaseBtn = new QHBoxLayout;
  mainDatabaseBtn->addStretch();
  mainDatabaseBtn->addWidget( m_updateDatabaseBtn );
  mainDatabaseBtn->addStretch();

  mainDatabaseLayout->addWidget( m_mainTableView );
  mainDatabaseLayout->addLayout( mainDatabaseBtn );

  grpMainTable->setLayout( mainDatabaseLayout );

  // User defined Database and btns
  QGroupBox *grpUserTable = new QGroupBox( "User database" );
  m_userTableView         = new QTableView;
  m_userTableView->setModel( new SensorDatabaseModel );
  m_userTableView->setSelectionBehavior( QAbstractItemView::SelectRows );

  QVBoxLayout *userDatabaseLayout = new QVBoxLayout;
  userDatabaseLayout->setSpacing( 5 );
  userDatabaseLayout->setContentsMargins( 5, 5, 5, 5 );

  m_addSensorBtn = new QPushButton( "Add" );
  m_addSensorBtn->setDefault( false );
  m_removeSensorBtn = new QPushButton( "Remove" );
  m_removeSensorBtn->setDefault( false );
  m_removeSensorBtn->setEnabled( false );
  m_removeAllSensorsBtn = new QPushButton( "Clear all" );
  m_removeAllSensorsBtn->setEnabled( false );
  m_removeAllSensorsBtn->setDefault( false );

  QHBoxLayout *userDatabaseBtn = new QHBoxLayout;
  userDatabaseBtn->addStretch();
  userDatabaseBtn->addWidget( m_addSensorBtn );
  userDatabaseBtn->addWidget( m_removeSensorBtn );
  userDatabaseBtn->addWidget( m_removeAllSensorsBtn );
  userDatabaseBtn->addStretch();

  userDatabaseLayout->addWidget( m_userTableView );
  userDatabaseLayout->addLayout( userDatabaseBtn );

  grpUserTable->setLayout( userDatabaseLayout );

  // Both database widgets
  QVBoxLayout *databaseLayout = new QVBoxLayout;
  databaseLayout->addWidget( grpMainTable, 2 );
  databaseLayout->addWidget( grpUserTable, 1 );

  // Main Buttons
  QHBoxLayout *mainBtnLayout = new QHBoxLayout;
  m_closeBtn                 = new QPushButton( "Close" );
  m_closeBtn->setDefault( true );
  mainBtnLayout->addStretch();
  mainBtnLayout->addWidget( m_closeBtn );

  // Main layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setSpacing( 5 );
  mainLayout->setContentsMargins( 10, 10, 10, 10 );

  mainLayout->addLayout( databaseLayout );
  mainLayout->addLayout( mainBtnLayout );

  setLayout( mainLayout );
}

void SensorDatabaseDialog::makeConnections( void )
{
  connect( m_addSensorBtn, SIGNAL( clicked() ), this, SLOT( onWantToAddSensor() ) );
  connect( m_removeSensorBtn, SIGNAL( clicked() ), this, SLOT( onWantToRemoveSensor() ) );
  connect( m_removeAllSensorsBtn, SIGNAL( clicked() ), this, SLOT( onWantToClearSensors() ) );
  connect( m_updateDatabaseBtn, SIGNAL( clicked() ), this, SLOT( onWantToUpdateFromInternet() ) );
  connect( m_closeBtn, SIGNAL( clicked() ), this, SLOT( onOk() ) );
  connect( m_userTableView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ), this, SLOT( onUserDefinedSelectionChanged() ) );
}

} // namespace openMVG_gui