#ifndef _OPENMVG_OPENMVG_GUI_DIALOGS_SENSOR_DATABASE_DIALOG_HH_
#define _OPENMVG_OPENMVG_GUI_DIALOGS_SENSOR_DATABASE_DIALOG_HH_

#include <QDialog>
#include <QPushButton>
#include <QTableView>

namespace openMVG_gui
{
/**
 * @brief Dialog used to show and edit sensor database
 * It provides also a way to dowload it from internet
 *
 */
class SensorDatabaseDialog : public QDialog
{
public:
  SensorDatabaseDialog( QWidget *parent );

public slots:

  /**
   * @brief Set the Main Database From a File 
   * 
   * @param path Path of the main database file 
   */
  void setMainDatabaseFromFile( const std::string &path );

  /**
   * @brief Set the User Defined Database From a file 
   * 
   * @param path Path of the user database file 
   */
  void setUserDefinedDatabaseFromFile( const std::string &path );

private slots:

  void onOk( void );

  /**
   * @brief When user click on the internet update btn
   *
   */
  void onWantToUpdateFromInternet( void );

  /**
   * @brief When user want to add a new sensor to the database
   *
   */
  void onWantToAddSensor( void );

  /**
   * @brief When user want to remove a sensor 
   * 
   */
  void onWantToRemoveSensor( void );

  /**
   * @brief When user want to clear all sensors 
   * 
   */
  void onWantToClearSensors( void );

  /**
   * @brief When user click on the user defined db
   * 
   */
  void onUserDefinedSelectionChanged( void );

private:
  void buildInterface( void );
  void makeConnections( void );

  QPushButton *m_addSensorBtn;
  QPushButton *m_removeSensorBtn;
  QPushButton *m_removeAllSensorsBtn;
  QPushButton *m_updateDatabaseBtn;
  QPushButton *m_closeBtn;

  QTableView *m_mainTableView;
  QTableView *m_userTableView;

  std::string m_userDatabaseFile;

  Q_OBJECT
};

} // namespace openMVG_gui

#endif