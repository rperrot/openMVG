#ifndef _OPENMVG_OPENMVG_GUI_DIALOGS_ADD_SENSOR_ENTRY_DIALOG_HH_
#define _OPENMVG_OPENMVG_GUI_DIALOGS_ADD_SENSOR_ENTRY_DIALOG_HH_

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

namespace openMVG_gui
{

/**
  * @brief Small dialog used to add a new sensor entry 
  * 
  */
class AddSensorEntryDialog : public QDialog
{
public:
  AddSensorEntryDialog( QWidget *parent );

  /**
   * @brief Get brand 
   * 
   * @return brand
   */
  std::string brand( void ) const;

  /**
   * @brief Get Model 
   * 
   * @return model
   */
  std::string model( void ) const;

  /**
   * @brief Sensor width
   * 
   * @return sensor width
   */
  double sensorWidth( void ) const;

public slots:

  /**
   * @brief action to be executed when user click on cancel button
   */
  void onCancel( void );

  /**
   * @brief action to be executed when user click on OK button
   */
  void onOk( void );

private:
  void buildInterface( void );
  void makeConnections( void );

  QLineEdit *m_brandTextEdit;
  QLineEdit *m_modelTextEdit;
  QLineEdit *m_sensorWidthTextEdit;

  QPushButton *m_ok;
  QPushButton *m_cancel;

  Q_OBJECT
};
} // namespace openMVG_gui

#endif