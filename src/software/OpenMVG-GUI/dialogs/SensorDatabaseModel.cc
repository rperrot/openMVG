#include "SensorDatabaseModel.hh"

#include <iostream>

namespace openMVG_gui
{
SensorDatabaseModel::SensorDatabaseModel( const std::string &sensorDatabasePath )
{
  std::vector<Datasheet> tmpData;
  const bool             ok = parseDatabase( sensorDatabasePath, tmpData );
  // todo : if( ! ok ) -> throw something

  for ( const auto &it : tmpData )
  {
    const auto        stop  = it.model_.find( ' ' );
    const std::string maker = it.model_.substr( 0, stop );
    std::string       model;
    if ( stop != std::string::npos )
    {
      model = it.model_.substr( it.model_.find( ' ' ) );
    }

    m_data.emplace_back( std::make_tuple( maker, model, it.sensorSize_ ) );
  }
}

SensorDatabaseModel::SensorDatabaseModel( void )
{
}

/**
 * @brief Get number of items in the sensor database
 *
 * @param parent
 * @return Number of element in the model
 */
int SensorDatabaseModel::rowCount( const QModelIndex &parent ) const
{
  return m_data.size();
}

/**
 * @brief Get number of components to show in the sensor database
 *
 * @param parent
 * @return Number of columns to show
 */
int SensorDatabaseModel::columnCount( const QModelIndex &parent ) const
{
  return 3;
}

/**
 * @brief Get an element of the table
 *
 * @param index
 * @param role
 * @return QVariant
 */
QVariant SensorDatabaseModel::data( const QModelIndex &index, int role ) const
{
  const int row = index.row();
  const int col = index.column();

  if ( row >= m_data.size() )
  {
    return QVariant();
  }
  if ( !index.isValid() )
  {
    return QVariant();
  }
  if ( role != Qt::DisplayRole )
  {
    return QVariant();
  }

  const std::tuple<std::string, std::string, double> &data = m_data[ row ];

  if ( col == 0 )
  {
    return QString( std::get<0>( data ).c_str() );
  }
  else if ( col == 1 )
  {
    return QString( std::get<1>( data ).c_str() );
  }
  else if ( col == 2 )
  {
    return std::get<2>( data );
  }
  else
  {
    return QVariant();
  }
}

/**
 * @brief Get header data
 *
 * @param section     Section
 * @param orientation Orientation of the header
 * @param role        Role of the data
 * @return            Header data at specific spection in the given orientation
 */
QVariant SensorDatabaseModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role != Qt::DisplayRole )
  {
    return QVariant();
  }
  if ( orientation == Qt::Horizontal )
  {
    if ( section == 0 )
    {
      return QString( "Brand" );
    }
    else if ( section == 1 )
    {
      return QString( "Model" );
    }
    else if ( section == 2 )
    {
      return QString( "Sensor width (in mm)" );
    }
    else
    {
      return QVariant();
    }
  }
  else
  {
    return section;
  }
}

/**
 * @brief Export database to a file
 *
 * @param path    Path of the output
 * @return true   If success
 * @return false  If failure
 */
bool SensorDatabaseModel::save( const std::string &path ) const
{
  std::ofstream file( path );
  if ( !file )
  {
    return false;
  }

  for ( const auto &it : m_data )
  {
    const std::string brand        = std::get<0>( it );
    const std::string model        = std::get<1>( it );
    const double      sensor_width = std::get<2>( it );

    file << brand << " " << model << ";" << std::fixed << sensor_width << "\n";
  }

  return true;
}

/**
   * @brief Append a new element to the model 
   * 
   * @param brand 
   * @param model 
   * @param size 
   */
void SensorDatabaseModel::append( const std::string &brand, const std::string &model, const double size )
{
  beginInsertRows( QModelIndex(), m_data.size(), m_data.size() );
  m_data.emplace_back( std::make_tuple( brand, model, size ) );
  endInsertRows();
}

/**
   * @brief Remove a set of rows from the model 
   * 
   * @param row       Id of the first row to remove s
   * @param count     Number of row to remove 
   * @param parent    (unused)
   * @return true     if success 
   * @return false    if failure
   */
bool SensorDatabaseModel::removeRows( int row, int count, const QModelIndex &parent )
{
  std::cerr << "Removing " << count << " rows, starting from : " << row << std::endl;
  if ( row + count <= m_data.size() )
  {
    beginRemoveRows( parent, row, row + count - 1 );
    m_data.erase( m_data.begin() + row, m_data.begin() + row + count );
    endRemoveRows();

    return true;
  }
  return false;
}

} // namespace openMVG_gui