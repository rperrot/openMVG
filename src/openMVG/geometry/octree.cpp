// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "octree.hpp"

#include <algorithm>
#include <limits>

namespace openMVG
{
namespace geometry
{

/**
 * @brief Constructor
 * @brief create an empty leaf
 */
OctreeNode::OctreeNode( void )
  : m_child
{
  nullptr , nullptr , nullptr , nullptr ,
  nullptr , nullptr , nullptr , nullptr
}

{

}

/**
 * @brief Dtr
 */
OctreeNode::~OctreeNode( void )
{
  for( int i = 0 ; i < 8 ; ++i )
  {
    delete m_child[i] ;
  }
}

/**
 * @brief Indicate if node is a leaf
 * @retval true if it's a leaf
 * @retval false if it's an internal node
 */
bool OctreeNode::isLeaf( void ) const
{
  for( int i = 0 ; i < 8 ; ++i )
  {
    if( m_child[ i ] )
    {
      return false ;
    }
  }
  return true ;
}

/**
 * @brief Get all indices in that node
 * @return list of indices
 */
std::vector<uint32_t> OctreeNode::indices( void ) const
{
  return m_indices ;
}

/**
 * @brief Get all indices in the node (including all the children's indices)
 * @return All indices in the node
 */
std::vector<uint32_t> OctreeNode::totalIndices( void ) const
{
  if( isLeaf() )
  {
    return m_indices ;
  }
  else
  {
    std::vector<uint32_t> res ;
    for( int i = 0 ; i < 8 ; ++i )
    {
      if( m_child[i] )
      {
        const std::vector<uint32_t> child_indices = m_child[i]->totalIndices() ;
        res.insert( res.end() , child_indices.begin() , child_indices.end() );
      }
    }
    return res ;
  }
}


/**
 * @brief Get number of indices in that node
 * @return The number of indices in that node
 */
uint32_t OctreeNode::nbIndices( void ) const
{
  return m_indices.size() ;
}

/**
 * @brief Return maximum depth of this node
 * @note by convention leaf has depth 0
 */
uint32_t OctreeNode::maxDepth( void ) const
{
  if( isLeaf() )
  {
    return 0 ;
  }
  else
  {
    uint32_t children_max_depth = 0 ;
    for( int i = 0 ; i < 8 ; ++i )
    {
      if( m_child[i] )
      {
        children_max_depth = std::max( children_max_depth , m_child[i]->maxDepth() ) ;
      }
    }
    return 1 + children_max_depth ;
  }
}

/**
 * @brief Get total number of element in the node (including it's children's elements)
 */
uint32_t OctreeNode::nbTotalElement( void ) const
{
  if( isLeaf() )
  {
    return nbIndices() ;
  }
  else
  {
    uint32_t children_nb_elt = 0 ;
    for( int i = 0 ; i < 8 ; ++i )
    {
      if( m_child[i] )
      {
        children_nb_elt += m_child[i]->nbTotalElement() ;
      }
    }
    return children_nb_elt ;
  }
}

/**
 * @brief Compute children bounds given node bounds
 * @param node_bounds The bounds of the current node
 * @param[out] The bounds of the children
 */
void OctreeNode::childrenBounds( const AABB & node_bounds ,
                                 std::array<AABB, 8> & children_bounds ) const
{
  // min ----- min + center ----- max
  const openMVG::Vec3 center = node_bounds.center() ;
  const openMVG::Vec3 delta = node_bounds.extends() / 2.0 ;
  const openMVG::Vec3 delta_x = openMVG::Vec3( delta.x() , 0 , 0 ) ;
  const openMVG::Vec3 delta_y = openMVG::Vec3( 0 , delta.y() , 0 ) ;
  const openMVG::Vec3 delta_z = openMVG::Vec3( 0 , 0 , delta.z() ) ;

  children_bounds[0] = AABB( node_bounds.min() , center ) ;
  children_bounds[1] = AABB( node_bounds.min() + delta_x , center + delta_x ) ;
  children_bounds[2] = AABB( node_bounds.min() + delta_y , center + delta_y ) ;
  children_bounds[3] = AABB( node_bounds.min() + delta_z , center + delta_z ) ;
  children_bounds[4] = AABB( node_bounds.min() + delta_x + delta_y , center + delta_x + delta_y ) ;
  children_bounds[5] = AABB( node_bounds.min() + delta_x + delta_z , center + delta_x + delta_z ) ;
  children_bounds[6] = AABB( node_bounds.min() + delta_y + delta_z , center + delta_y + delta_z ) ;
  children_bounds[7] = AABB( delta , node_bounds.max() ) ;
}


/**
 * @brief Constructor
 * @param bounds Bounds of the octree
 * @param max_depth Maximum depth of the octree
 * @param max_point_in_leaf Maximum number of points in the nodes
 * @note Maximum is a bound that indicate when to split the data
 */
PointCloudOctree::PointCloudOctree( const std::vector<element_t> & pts ,
                                    const uint32_t max_depth  ,
                                    const uint32_t max_point_in_leaf )
  : m_root( nullptr ) ,
    m_bounds( pts ) ,
    m_pts( pts ) ,
    m_max_depth( max_depth ) ,
    m_max_point_in_leaf( max_point_in_leaf )
{

}

/**
 * @brief Dtr
 */
PointCloudOctree::~PointCloudOctree( void )
{
  delete m_root ;
}

/**
 * @brief Get maximum depth of the tree
 * @return Maximum depth of the octree
 */
uint32_t PointCloudOctree::maxDepth() const
{
  if( ! m_root )
  {
    return std::numeric_limits<uint32_t>::max() ;
  }
  else
  {
    return m_root->maxDepth() ;
  }
}

/**
 * @brief Get number of element in the octree
 * @return number of element in the octree
 */
uint32_t PointCloudOctree::nbElement() const
{
  if( ! m_root )
  {
    return 0 ;
  }
  else
  {
    return m_root->nbTotalElement() ;
  }
}

/**
 * @brief Perform culling of the data using a bounding box
 * @param valid_space Space in which data is searched
 * @return List of id that are in the valid_space
 */
std::vector<PointCloudOctree::key_t> PointCloudOctree::cull( const AABB & valid_space ) const
{
  if( ! m_root )
  {
    return {} ;
  }
  else
  {
    return cull( m_root , m_bounds , valid_space ) ;
  }
}

/**
 * @brief Perform culling of the data using frustum
 * @param valid_space Space in which data is searched
 * @return List of id that are in the valid_space
 */
std::vector<PointCloudOctree::key_t> PointCloudOctree::cull( const Frustum & valid_space ) const
{
  if( ! m_root )
  {
    return {} ;
  }
  else
  {
    return cull( m_root , m_bounds , valid_space ) ;
  }
}



/**
 * @brief Perform insertion inside the octree
 * @param node node of the box
 * @param box bound of the current box
 * @param depth current depth of insertion
 * @param pt Point to insert
 * @retval true if insertion could be performed
 * @retval false if an error occurs
 */
bool PointCloudOctree::insert( OctreeNode * node , const AABB & box , const uint32_t depth , const std::pair< key_t , element_t > & pt )
{
  if( depth >= m_max_depth )
  {
    node->m_indices.emplace_back( pt.first ) ;
    return true ;
  }

  if( node->isLeaf() )
  {
    if( node->nbIndices() < m_max_point_in_leaf )
    {
      node->m_indices.emplace_back( pt.first ) ;
      return true ;
    }
    else
    {
      // Split node
      splitNode( node , box , depth ) ;

      // Insert in the good child (but let the node choose)
      return insert( node , box , depth , pt ) ;
    }
  }
  else
  {
    // Internal node, choose where to go
    std::array<AABB, 8> child_boxes ;
    node->childrenBounds( box , child_boxes ) ;

    for( int id_child = 0 ; id_child < 8 ; ++id_child )
    {
      if( child_boxes[id_child].contains( pt.second ) )
      {
        if( ! node->m_child[id_child] )
        {
          node->m_child[id_child] = new OctreeNode ;
        }
        return insert( node->m_child[id_child] , child_boxes[id_child] , depth + 1 , pt ) ;
      }
    }
    // If here the point hasnot been inserted
    return false ;
  }
}

/**
* @brief Given a leaf split it and dispatch its points into its children
* @param node The node to split
* @param box The bounding box of the node
*/
void PointCloudOctree::splitNode( OctreeNode * node , const AABB & box , const uint32_t depth )
{
  std::array<AABB, 8> child_boxes ;
  node->childrenBounds( box , child_boxes ) ;

  // Dispatch node points into the childs
  for( const uint32_t cur_id : node->m_indices )
  {
    const openMVG::Vec3 & cur_pt = m_pts[ cur_id ] ;

    for( int id_child = 0 ; id_child < 8 ; ++id_child )
    {
      if( child_boxes[id_child].contains( cur_pt ) )
      {
        // On demand creation
        if( ! node->m_child[id_child] )
        {
          node->m_child[id_child] = new OctreeNode ;
        }
        insert( node->m_child[id_child] , child_boxes[id_child] , depth + 1 , std::make_pair( cur_id , cur_pt ) ) ;
        break ;
      }
    }
  }

  // Remove the indices
  std::vector<uint32_t> tmp ;
  std::swap( tmp , node->m_indices ) ;
}

/**
 * @brief Cull the node data according to a valid space
 * @param node The current node to cull
 * @param node_box The current node bounding box
 * @param valid_space The search space
 * @return List of ids that are in the search space
 */
std::vector<PointCloudOctree::key_t> PointCloudOctree::cull( const OctreeNode * node , const AABB & node_box , const AABB & valid_space ) const
{
  if( node->isLeaf() )
  {
    std::vector<key_t> result ;
    for( const uint32_t id_pt : node->m_indices )
    {
      const openMVG::Vec3 & cur_pt = m_pts[ id_pt ] ;

      if( valid_space.contains( cur_pt ) )
      {
        result.emplace_back( id_pt ) ;
      }
    }
    return result ;
  }
  else
  {
    // Internal node
    std::array<AABB, 8> child_bounds ;
    node->childrenBounds( node_box , child_bounds ) ;

    std::vector<key_t> result ;
    for( int id_child = 0 ; id_child < 8 ; ++id_child )
    {
      if( node->m_child[id_child] )
      {
        if( valid_space.contains( child_bounds[id_child] ) )
        {
          // Fully contains -> get all indices
          const std::vector<key_t> tmp = node->m_child[ id_child ]->totalIndices() ;
          result.insert( result.end() , tmp.begin() , tmp.end() ) ;
        }
        else if( valid_space.overlap( child_bounds[id_child] ) )
        {
          // Partially intersect -> cull the child
          const std::vector<key_t> tmp = cull( node->m_child[id_child] , child_bounds[id_child] , valid_space ) ;
          result.insert( result.end() , tmp.begin() , tmp.end() ) ;
        }
        // else -> no intersection : stop
      }
    }
    return result ;
  }
}


/**
 * @brief Cull the node data according to a valid space
 * @param node The current node to cull
 * @param node_box The current node bounding box
 * @param valid_space The search space
 * @return List of ids that are in the search space
 */
std::vector<PointCloudOctree::key_t> PointCloudOctree::cull( const OctreeNode * node , const AABB & node_box , const Frustum & valid_space ) const
{
  if( node->isLeaf() )
  {
    std::vector<key_t> result ;
    for( const uint32_t id_pt : node->m_indices )
    {
      const openMVG::Vec3 & cur_pt = m_pts[ id_pt ] ;

      if( valid_space.contains( cur_pt ) )
      {
        result.emplace_back( id_pt ) ;
      }
    }
    return result ;
  }
  else
  {
    // Internal node
    std::array<AABB, 8> child_bounds ;
    node->childrenBounds( node_box , child_bounds ) ;

    std::vector<key_t> result ;
    for( int id_child = 0 ; id_child < 8 ; ++id_child )
    {
      if( node->m_child[id_child] )
      {
        if( child_bounds[id_child].containedIn( valid_space ) )
        {
          // Fully contains -> get all indices
          const std::vector<key_t> tmp = node->m_child[ id_child ]->totalIndices() ;
          result.insert( result.end() , tmp.begin() , tmp.end() ) ;
        }
        else if( child_bounds[id_child].overlap( valid_space ) )
        {
          // Partially intersect -> cull the child
          const std::vector<key_t> tmp = cull( node->m_child[id_child] , child_bounds[id_child] , valid_space ) ;
          result.insert( result.end() , tmp.begin() , tmp.end() ) ;
        }
        // else -> no intersection : stop
      }
    }
    return result ;
  }
}



} // namespace geometry
} // namespace openMVG
