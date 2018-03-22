// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_GEOMETRY_AABB_HPP
#define OPENMVG_GEOMETRY_AABB_HPP

#include "openMVG/numeric/eigen_alias_definition.hpp"
#include "openMVG/geometry/frustum.hpp"

namespace openMVG
{
namespace geometry
{

/**
 * @brief Class holding an Axis Aligned Bounding Box (AABB) in 3d
 * This class is used to compute frustum culling (either directly or using an octree)
 */
class AABB
{
  public:

    /**
     * @brief Construct a null bounding box
     */
    AABB( void ) ;

    /**
     * @brief Construct a bbox that perfectly fit the set of points
     * @param pts The points
     */
    AABB( const std::vector<openMVG::Vec3> & pts ) ;


    AABB( const AABB & src ) = default ;
    AABB( AABB && src ) = default ;

    AABB & operator=( const AABB & src ) = default ;
    AABB & operator=( AABB && src ) = default ;

    /**
     * @brief Construct a bounding box given minimum and maximum extends
     * @param min Minimum extend of the bounding box
     * @param max Maximum extend of the bounding box
     */
    AABB( const openMVG::Vec3 & min , const openMVG::Vec3 & max ) ;

    /**
     * @brief Indicate if the bounding box is empty
     * @retval true if the box is empty
     * @retval false if the box is not empty
     */
    bool empty( void ) const ;

    /**
     * @brief Get extends in each dimensions
     * @return extend in X,Y,Z of each dimensions
     * @note If bounding box is empty, return { -1 , -1 , -1 }
     */
    openMVG::Vec3 extends( void ) const ;

    /**
     * @brief Get center of the bounding box
     * @return Center of the bounding box
     * @note If bounding box is empty, return { Inf , Inf , Inf } ;
     */
    openMVG::Vec3 center( void ) const ;

    /**
     * @brief Get minimum coordinate of the bounding box
     * @return Minimum coordinate of the bounding box
     */
    openMVG::Vec3 min( void ) const ;

    /**
     * @brief Get maximum coordinate of the bounding box
     * @return Maximum coordinate of the bounding box
     */
    openMVG::Vec3 max( void ) const ;

    /**
     * @brief Update the bounding box such as it contains the given point
     * @param pt The point that will be inside the bounding box after update
     */
    void update( const openMVG::Vec3 & pt ) ;

    /**
     * @brief Update the bounding box such as it contains the given pointset
     * @param pts The points that will be inside the bounding box after update
     */
    void update( const std::vector< openMVG::Vec3 > & pts ) ;

    /**
     * @brief Update the bounding box such as it contains the bounding box
     * @param src The bounding box used to update the bounding box
     */
    void update( const AABB & src ) ;

    /**
     * @brief Indicate if the bounding box contains the point
     * @param pt The point to test
     * @retval true If the point is inside the bounding box
     * @retval false If the point is outside the bounding box
     */
    bool contains( const openMVG::Vec3 & pt ) const ;

    /**
     * @brief Indicate if the bounding box fully contains the given box
     * @param box The queried bounding box
     * @retval true If the box is (fully) inside the bounding box
     * @retval false If the box is outside the bounding box or partially overlap
     */
    bool contains( const AABB & box ) const ;


    /**
     * @brief Indicate if the bounding box fully contains the triangle
     * @param A first vertex of the triangle
     * @param B second vertex of the triangle
     * @param C third vertex of the triangle
     * @retval true if the triangle is fully inside the bounding box
     * @retval false if the triangle is not fully (perhaps partially) inside the bounding box
     */
    bool contains( const openMVG::Vec3 & A ,
                   const openMVG::Vec3 & B ,
                   const openMVG::Vec3 & C ) const ;


    /**
     * @brief Indicate if the bounding box has non null intersection with the given one
     * @param box Bounding box to test
     * @retval true if the given box has non null intersection with the current box
     * @retval false if the given box has no intersection with the current box
     */
    bool overlap( const AABB & box ) const ;

    /**
     * @brief Indicate if the bounding box has non null intersection with the triangle
     * @param A first vertex of the triangle
     * @param B second vertex of the triangle
     * @param C third vertex of the triangle
     * @retval true if the triangle has intersection with the bounding box
     * @retval false if the triangle has no intersection with the bounding box
     */
    bool overlap( const openMVG::Vec3 &A ,
                  const openMVG::Vec3 & B ,
                  const openMVG::Vec3 & C ) const ;


    /**
     * @brief Indicate if AABB is fully contained in the frustum
     * @param fr Frustum
     * @retval true if the box is fully contained in the Frustum
     * @retval false if no intersection or only partially
     */
    bool containedIn( const Frustum & fr ) const ;

    /**
     * @brief Indicate if the Frustum and the box have non empty intersection
     * @param fr Frustum
     * @retval true if the frustum and the bounding box have non empty intersection
     * @retval false if no intersection
     */
    bool overlap( const Frustum & fr ) const ;

  private:

    /// Minimum extend of the bounding box
    openMVG::Vec3 m_min ;
    /// Maximum extend of the bounding box
    openMVG::Vec3 m_max ;
} ;

} // namespace geometry
} // namespace openMVG

#endif