#include "PointCloud.hpp"

#include "Util.hpp"

#include <fstream>

namespace MVS
{
  /**
  * @brief Ctr
  */
  PointCloud::PointCloud( void )
  {

  }

  /**
  * @brief Add a point to the point cloud
  */
  void PointCloud::AddPoint( const openMVG::Vec3 & vec , const openMVG::Vec3 & n , const openMVG::Vec3 & c )
  {
    m_pts.push_back( vec ) ;
    m_normals.push_back( n ) ;
    m_colors.push_back( c ) ;
  }

  /**
  * @brief Export point cloud to a ply point file
  */
  bool PointCloud::ExportToPly( const std::string & path , const bool export_binary ) const
  {
    std::ofstream file( path ) ;
    if( ! file )
    {
      std::cerr << "Could not create PLY file : \'" << path << "\'" << std::endl ;
      return false ;
    }

    file << "ply" << std::endl ;

    // Header
    if( export_binary )
    {
      // TODO be less restrictive because host system may not be a little endian system
      file << "format binary_little_endian 1.0" << std::endl ;
    }
    else
    {
      file << "format ascii 1.0" << std::endl ;
    }

    file << "element vertex " << m_pts.size() << std::endl ;
    file << "property float32 x" << std::endl ;
    file << "property float32 y" << std::endl ;
    file << "property float32 z" << std::endl ;
    file << "property float32 nx" << std::endl ;
    file << "property float32 ny" << std::endl ;
    file << "property float32 nz" << std::endl ;
    file << "property uchar red" << std::endl ;
    file << "property uchar green" << std::endl ;
    file << "property uchar blue" << std::endl ;
    file << "end_header" << std::endl ;

    if( export_binary )
    {
      // Reopen in binary mode at the end of the file
      file.close() ;
      file.open( path ,  std::ofstream::app | std::ofstream::binary | std::ofstream::out ) ;

      for( size_t id_pt = 0 ; id_pt < m_pts.size() ; ++id_pt )
      {
        const openMVG::Vec3 & cur_pt = m_pts[ id_pt ] ;
        const openMVG::Vec3 & cur_no = m_normals[ id_pt ] ;
        const openMVG::Vec3 & cur_col = m_colors[ id_pt ] ;

        const float tmp[] =
        {
          static_cast<float>( cur_pt[0] ) ,
          static_cast<float>( cur_pt[1] ) ,
          static_cast<float>( cur_pt[2] ) ,

          static_cast<float>( cur_no[0] ) ,
          static_cast<float>( cur_no[1] ) ,
          static_cast<float>( cur_no[2] )
        } ;
        const int r = static_cast<int>( cur_col[0] * 255.0 ) ;
        const int g = static_cast<int>( cur_col[1] * 255.0 ) ;
        const int b = static_cast<int>( cur_col[2] * 255.0 ) ;

        const unsigned char tmp_col[] =
        {
          static_cast<unsigned char>( MVS::Clamp( r , 0 , 255 ) ) ,
          static_cast<unsigned char>( MVS::Clamp( g , 0 , 255 ) ) ,
          static_cast<unsigned char>( MVS::Clamp( b , 0 , 255 ) )
        } ;

        file.write( reinterpret_cast<const char*>( tmp ) , 6 * sizeof( float ) ) ;
        file.write( reinterpret_cast<const char*>( tmp_col ) , 3 * sizeof( unsigned char ) ) ;
      }
    }
    else
    {
      for( size_t id_pt = 0 ; id_pt < m_pts.size() ; ++id_pt )
      {
        const openMVG::Vec3 & cur_pt = m_pts[ id_pt ] ;
        const openMVG::Vec3 & cur_no = m_normals[ id_pt ] ;
        const openMVG::Vec3 & cur_col = m_colors[ id_pt ] ;

        const int r = MVS::Clamp( static_cast<int>( cur_col[0] * 255.0 ) , 0 , 255 ) ;
        const int g = MVS::Clamp( static_cast<int>( cur_col[1] * 255.0 ) , 0 , 255 ) ;
        const int b = MVS::Clamp( static_cast<int>( cur_col[2] * 255.0 ) , 0 , 255 ) ;


        file << cur_pt[0] << " " << cur_pt[1] << " " << cur_pt[2] << " " ;
        file << cur_no[0] << " " << cur_no[1] << " " << cur_no[2] << " " ;
        file << r << " " << g << " " << b << std::endl ;
      }
    }

    return true ;
  }

  /**
  * @brief Get number of point in the cloud
  * @return of points
  */
  unsigned long PointCloud::NbPoint( void ) const
  {
    return m_pts.size() ;
  }

  /**
  * @brief Append a point cloud
  */
  void PointCloud::Append( const PointCloud & pcl )
  {
    m_pts.insert( m_pts.begin() , pcl.m_pts.begin() , pcl.m_pts.end() );
    m_normals.insert( m_normals.begin() , pcl.m_normals.begin() , pcl.m_normals.end() ) ;
    m_colors.insert( m_colors.begin() , pcl.m_colors.begin() , pcl.m_colors.end() ) ;
  }


}