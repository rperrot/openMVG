#ifndef _OPENMVG_OPENMVG_GUI_DIALOGS_SENSOR_DATABASE_MODEL_HH_
#define _OPENMVG_OPENMVG_GUI_DIALOGS_SENSOR_DATABASE_MODEL_HH_

#include <openMVG/exif/sensor_width_database/ParseDatabase.hpp>

#include <QAbstractTableModel>

#include <tuple>

namespace openMVG_gui
{
class SensorDatabaseModel : public QAbstractTableModel
{
public:
  SensorDatabaseModel( const std::string &sensorDatabasePath );

  SensorDatabaseModel( void );

  /**
   * @brief Get number of items in the sensor database
   *
   * @param parent
   * @return Number of element in the model
   */
  int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

  /**
   * @brief Get number of components to show in the sensor database
   *
   * @param parent
   * @return Number of columns to show
   */
  int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

  /**
   * @brief Get an element of the table
   *
   * @param index
   * @param role
   * @return QVariant
   */
  QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

  /**
   * @brief Get header data
   *
   * @param section     Section
   * @param orientation Orientation of the header
   * @param role        Role of the data
   * @return            Header data at specific spection in the given orientation
   */
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

  /**
   * @brief Export database to a file
   *
   * @param path    Path of the output
   * @return true   If success
   * @return false  If failure
   */
  bool save( const std::string &path ) const;

  /**
   * @brief Append a new element to the model 
   * 
   * @param brand 
   * @param model 
   * @param size 
   */
  void append( const std::string &brand, const std::string &model, const double size );

  /**
   * @brief Remove a set of rows from the model 
   * 
   * @param row       Id of the first row to remove s
   * @param count     Number of row to remove 
   * @param parent    (unused)
   * @return true     if success 
   * @return false    if failure
   */
  bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

private:
  std::vector<std::tuple<std::string, std::string, double>> m_data;
};
} // namespace openMVG_gui

#endif