// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_GEOMETRY_OCTREE_HPP
#define OPENMVG_GEOMETRY_OCTREE_HPP

#include "openMVG/geometry/AABB.hpp"

#include <array>
#include <utility>

namespace openMVG
{
namespace geometry
{

/**
 * @brief Class holding an octree node
 */
struct OctreeNode
{
  /**
   * @brief Constructor
   * @brief create an empty leaf
   */
  OctreeNode( void ) ;

  /**
   * @brief Dtr
   */
  ~OctreeNode( void ) ;

  /**
   * @brief Indicate if node is a leaf
   * @retval true if it's a leaf
   * @retval false if it's an internal node
   */
  bool isLeaf( void ) const ;

  /**
   * @brief Get all indices in that node
   * @return list of indices
   */
  std::vector<uint32_t> indices( void ) const ;

  /**
   * @brief Get all indices in the node (including all the children's indices)
   * @return All indices in the node
   */
  std::vector<uint32_t> totalIndices( void ) const ;

  /**
   * @brief Get number of indices in that node
   * @return The number of indices in that node
   */
  uint32_t nbIndices( void ) const ;

  /**
   * @brief Return maximum depth of this node
   * @note by convention leaf has depth 0
   */
  uint32_t maxDepth( void ) const ;

  /**
   * @brief Get total number of element in the node (including it's children's elements)
   * @return Number of element in the node
   */
  uint32_t nbTotalElement( void ) const ;

  /**
   * @brief Compute children bounds given node bounds
   * @param node_bounds The bounds of the current node
   * @param[out] The bounds of the children
   */
  void childrenBounds( const AABB & node_bounds , std::array<AABB, 8> & children_bounds ) const  ;

  /// Childs of the node
  OctreeNode * m_child[8] ;
  std::vector<uint32_t> m_indices ;
} ;

/**
 * @brief Basic Octree structure used to store points
 * @note this is used to store static point cloud
 * @note the points set (given in constructor) should be keep into memory as long as the octree is used
 */
class PointCloudOctree
{
  public:

    /// Key that identify the element to be added
    typedef uint32_t key_t ;
    /// The element associated with the key to add in the structure
    typedef openMVG::Vec3 element_t ;

    /**
     * @brief Constructor
     * @param pts List of points
     * @param max_depth Maximum depth of the octree
     * @param max_point_in_leaf Maximum number of points in the nodes
     * @note Maximum is a bound that indicate when to split the data
     */
    PointCloudOctree( const std::vector<element_t> & pts ,
                      const uint32_t max_depth = 10 ,
                      const uint32_t max_point_in_leaf = 32 ) ;

    /**
     * @brief Dtr
     */
    ~PointCloudOctree( void ) ;

    /**
     * @brief Get maximum depth of the tree
     * @return Maximum depth of the octree
     * @note if tree is empty, returns std::numeric_limits<uint32_t>::max()
     */
    uint32_t maxDepth() const ;

    /**
     * @brief Get number of element in the octree
     * @return number of element in the octree
     */
    uint32_t nbElement() const ;

    /**
     * @brief Perform culling of the data using a bounding box
     * @param valid_space Space in which data is searched
     * @return List of id that are in the valid_space
     */
    std::vector<key_t> cull( const AABB & valid_space ) const ;

    /**
     * @brief Perform culling of the data using frustum
     * @param valid_space Space in which data is searched
     * @return List of id that are in the valid_space
     */
    std::vector<key_t> cull( const Frustum & valid_space ) const ;

  private:

    /**
     * @brief Perform insertion inside the octree
     * @param node node of the box
     * @param box bound of the current box
     * @param depth current depth of insertion
     * @param pt Point to insert
     * @retval true if insertion could be performed
     * @retval false if an error occurs
     */
    bool insert( OctreeNode * node , const AABB & box , const uint32_t depth , const std::pair< key_t , element_t > & pt ) ;

    /**
     * @brief Given a leaf split it and dispatch its points into its children
     * @param node The node to split
     * @param box The bounding box of the node
     * @param depth Current depth of the node
     */
    void splitNode( OctreeNode * node , const AABB & box , const uint32_t depth ) ;

    /**
     * @brief Cull the node data according to a valid space
     * @param node The current node to cull
     * @param node_box The current node bounding box
     * @param valid_space The search space
     * @return List of ids that are in the search space
     */
    std::vector<key_t> cull( const OctreeNode * node , const AABB & node_box , const AABB & valid_space ) const ;

    /**
     * @brief Cull the node data according to a valid space
     * @param node The current node to cull
     * @param node_box The current node bounding box
     * @param valid_space The search space
     * @return List of ids that are in the search space
     */
    std::vector<key_t> cull( const OctreeNode * node , const AABB & node_box , const Frustum & valid_space ) const ;


    OctreeNode * m_root ;
    AABB m_bounds ;
    const std::vector<element_t> & m_pts ;

    uint32_t m_max_depth ;
    uint32_t m_max_point_in_leaf ;
} ;

} // namespace geometry
} // namespace openMVG

#endif