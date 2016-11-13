#ifndef _OPENMVG_MVS_POINT_CLOUD_HPP_
#define _OPENMVG_MVS_POINT_CLOUD_HPP_

#include "openMVG/numeric/numeric.h"

#include <string>
#include <vector>

namespace MVS
{
  /**
  * Simple exporter of point cloud
  */
  class PointCloud
  {
    public:
      /**
      * @brief Ctr
      */
      PointCloud( void ) ;

      /**
      * @brief Add a point to the point cloud
      * @param vec Point position
      * @param n Point normal
      * @param col Point color
      */
      void AddPoint( const openMVG::Vec3 & vec , const openMVG::Vec3 & n , const openMVG::Vec3 & col ) ;

      /**
      * @brief Export point cloud to a ply point file
      * @param path Path of the file to generate
      * @param export_binary Indicate if the ply file should be exported in binary format
      */
      bool ExportToPly( const std::string & path , const bool export_binary ) const ;

      /**
      * @brief Append a point cloud
      * @param pcl Point cloud to append to the current one
      */
      void Append( const PointCloud & pcl ) ;

      /**
      * @brief Get number of point in the cloud
      * @return of points
      */
      unsigned long NbPoint( void ) const ;

    private:

      std::vector< openMVG::Vec3 > m_pts ;
      std::vector< openMVG::Vec3 > m_normals ;
      std::vector< openMVG::Vec3 > m_colors ;
  } ;
}

#endif