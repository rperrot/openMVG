#ifndef OPENMVG_GRAPH_GRAPH_IO_HPP_
#define OPENMVG_GRAPH_GRAPH_IO_HPP_

#include <cereal/archives/portable_binary.hpp>

#include <string>
#include <fstream>

namespace openMVG
{
namespace graph
{

/**
* @brief Load a graph from disk
* @param[out] h Output graph
* @param[in] file_name Path where graph is loaded
* @retval true If loading could be made correctly
* @retval false If there was an error during loading
* @note NodeType and EdgeType must be default constructible
* @note if NodeType and/or EdgeType are pointers node data and edge data is undefined
*/
template< typename NodeType , typename EdgeType>
bool Load( UndirectedGraph<NodeType, EdgeType> & g , const std::string & file_name ) ;

/**
* @brief Save a graph to the disk
* @param[in] g Input graph to write to disk
* @param[in] file_name Output path where graph is stored
* @retval true If save could be made correctly
* @retval false If there was an error during saving
* @note If NodeType and/or EdgeType are pointers node data and edge data storage is unusable for future loading
*/
template< typename NodeType , typename EdgeType>
bool Save( const UndirectedGraph<NodeType, EdgeType> & g , const std::string & file_name ) ;


/**
* @brief Load a graph from disk
* @param[out] h Output graph
* @param[in] file_name Path where graph is loaded
* @retval true If loading could be made correctly
* @retval false If there was an error during loading
* @note NodeType and EdgeType must be default constructible
* @note if NodeType and/or EdgeType are pointers node data and edge data is undefined
*/
template< typename NodeType , typename EdgeType>
bool Load( UndirectedGraph<NodeType, EdgeType> & g , const std::string & file_name )
{
  std::ifstream stream( file_name , std::ios::binary ) ;
  if( ! stream.good() )
  {
    return false ;
  }

  cereal::PortableBinaryInputArchive archive( stream ) ;
  archive( g ) ;
  return true ;
}

/**
* @brief Save a graph to the disk
* @param[in] g Input graph to write to disk
* @param[in] file_name Output path where graph is stored
* @retval true If save could be made correctly
* @retval false If there was an error during saving
* @note If NodeType and/or EdgeType are pointers node data and edge data storage is unusable for future loading
*/
template< typename NodeType , typename EdgeType>
bool Save( const UndirectedGraph<NodeType, EdgeType> & g , const std::string & file_name )
{
  std::ofstream stream( file_name , std::ios::binary ) ;
  if( ! stream.good() )
  {
    return false ;
  }

  cereal::PortableBinaryOutputArchive archive( stream ) ;

  archive( g ) ;

  return true ;
}

}
}

#endif