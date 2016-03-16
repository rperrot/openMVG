// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_PAIRING_HEAP_HPP_
#define _OPENMVG_GRAPH_PAIRING_HEAP_HPP_

#include <stack>

namespace openMVG
{
namespace graph
{

/**
* @brief Node in a pairing heap implementation
* @note This should never been used directly, use the PairingHeap structure
*/
template< typename KeyType, typename DataType >
struct PairingNode
{
  // Pointer to the first child
  PairingNode<KeyType, DataType> * m_child ;

  // Pointer to the left element or if we are first, to the parent
  PairingNode<KeyType, DataType> * m_prev ;

  // Pointer to the right element or nullptr if we are the last
  PairingNode<KeyType, DataType> * m_next ;


  KeyType m_key ;
  DataType m_value ;
} ;

/**
* @brief Implements a heap structure with following operations
* Creation -> Create an empty heap
* Insertion( key , data ) -> Insert element in the heap
* FindMin() -> Find element with minimum key
* DeleteMin() -> Delete element with minimum key
* DecreaseKey( elt , newKey ) -> decrease key value of a specified element
* Complexity is given as follow :
* Creation -> O(n) where n is the max number of elements
* FindMin() -> O(1)
* DeleteMin() -> Amortized O(ln n)
* DecreaseKey() -> Amortized O(ln ln n)
* Note this theorical complexity is less than Fibonacci trees but in practice PH is faster due to simpler structure
*/
template< typename KeyType, typename DataType >
class PairingHeap
{
  public:

    /// Type of the internal nodes
    typedef PairingNode<KeyType, DataType> node_type ;

    /**
    * @brief Create a new heap
    * @param max_nb_elt Maximum number of element to be inserted in the heap
    * @note We do not support unlimited element in the heap,
    * practically this should never be an issue, since it's often used in graph algorithms and bounded by the number of nodes/edges
    */
    PairingHeap( const int max_nb_elt ) ;

    /**
    * @brief Dtor
    */
    ~PairingHeap( void ) ;

    /**
    * @brief Get key associated with a node
    * @param elt Node to query
    * @return key associated to the node
    */
    KeyType GetKey( PairingNode<KeyType, DataType> * elt ) ;

    /**
    * @brief Get data associated with a node
    * @param elt Node to query
    * @return Data associated to the node
    */
    DataType GetData( PairingNode<KeyType, DataType> * elt ) ;

    /**
    * @brief Insert an element inside the heap
    * @param key Key of the new element
    * @param data Data of the new element
    * @return a pointer to The newly created element
    */
    PairingNode<KeyType, DataType> * Insert( const KeyType key , const DataType data ) ;

    /**
    * @brief Get element of minimum key value
    * @return Pointer to the heap element with minimum value
    * @note If heap is empty, return nullptr
    */
    PairingNode<KeyType, DataType> * FindMin( ) ;

    /**
     * @brief Delete element with minimum key value
     * @note If heap is empty, do nothing
     */
    void DeleteMin( void ) ;

    /**
    * @brief Get the number element in the heap
    * @return number of element in the heap
    * @note This is the currently active element in the heap, not the maximum size
    */
    unsigned long Size( void ) ;

    /**
    * @brief Decrease key value of a specified element
    * @param elt Element to decrease
    * @param key the new key value
    */
    void DecreaseKey( PairingNode<KeyType, DataType> * elt , const KeyType key ) ;

    /**
    * @brief Remove an element from the heap
    * @param elt Element to remove
    * @note After removal, pointed value has no meaning
    */
    void Delete( PairingNode<KeyType, DataType> * elt ) ;

    /**
    * @brief Tests if the heap is empty
    * @retval true if the heap is empty
    * @retval false if the heap contains at least one element
    */
    bool Empty( void ) const ;

  private:

    unsigned long m_nb_elt ;

    PairingNode<KeyType, DataType> * m_nodes ;

    std::stack< PairingNode<KeyType, DataType> * > m_avail_nodes ;

    // Root of the tree
    PairingNode<KeyType, DataType> * m_root ;
} ;

/**
* @brief Merge two heaps into a new heap
* @param a a heap
* @param b a heap
* @return a pointer to the root of the new heap
*/
template< typename KeyType, typename DataType>
static inline PairingNode<KeyType, DataType> * Merge( PairingNode<KeyType, DataType> * a , PairingNode<KeyType, DataType> * b )
{
  // Limit cases, some of the node is nullptr
  if( a == nullptr )
  {
    return b ;
  }
  else if( b == nullptr )
  {
    return a ;
  }

  if( a == b ) // Merge one node with itself
  {
    return a ;
  }

  // Find which node is the parent of the other
  const bool b_min = b->m_key < a->m_key ;
  PairingNode<KeyType, DataType> * parent = ( b_min ) ? b : a ;
  PairingNode<KeyType, DataType> * child = ( b_min ) ? a : b ;

  // Now update connections to make relation parent -> child
  child->m_next = parent->m_child ;
  if( parent->m_child )
  {
    // Make the child at the same level as the child of the parent
    parent->m_child->m_prev = child ;
  }

  // Update child's parent
  child->m_prev = parent ;
  // Update parent's child
  parent->m_child = child ;

  // Parent has only child, no left of right element
  parent->m_next = nullptr ;
  parent->m_prev = nullptr ;

  return parent ;
}

// Perform merging of list to get a tree (used when delete)
// Two pass
// 1st) Takes all the child element from left to right, merge then by pairs
// 2nd) Takes all the pairs from right to left, merge then in one heap
// return the root node of the final heap
// head may be the first child of an element to remove
template< typename KeyType, typename DataType>
static inline PairingNode<KeyType, DataType> * TwoPassMerging( PairingNode<KeyType, DataType> * head )
{
  if( head == nullptr )
  {
    return nullptr ;
  }

  // Get head and tail of the list
  PairingNode<KeyType, DataType> * next = head ;
  PairingNode<KeyType, DataType> * tail = nullptr ;

  // First pass -> Make pairs left to right
  while ( next )
  {
    // And element and it's successor
    PairingNode<KeyType, DataType> * a = next ;
    PairingNode<KeyType, DataType> * b = a->m_next ;

    // Not the end of the list
    if ( b )
    {
      next = b->m_next; // Store the next element to process

      // Merge the two elements and make a link to join them
      PairingNode<KeyType, DataType> * tmp = Merge( a , b );
      tmp->m_prev = tail ;
      tail = tmp ;
    }
    else // a is the last item
    {
      a->m_prev = tail ;
      tail = a ;
      break;
    }
  }
  // Here tail is the last element of the list. 2nd pass starts with that element

  // Pointer to the root of the newly created heap
  PairingNode<KeyType, DataType> * res = nullptr ;

  // Merge hierarchy to ensure heap
  // Combine right to left (from tail to end of the list)
  while ( tail )
  {
    // Store previous for next iteration
    next = tail->m_prev ;

    // Merge consecutives elements in a heap
    res = Merge( res , tail ) ;

    // Update tail to work on the previous (ie: left) element
    tail = next ;
  }

  // get the new head
  return res;
}



/**
* @brief Create a new heap
* @param max_nb_elt Maximum number of element to be inserted in the heap
* @note We do not support unlimited element in the heap,
* practically this should never be an issue, since it's often used in graph algorithms and bounded by the number of nodes/edges
*/
template< typename KeyType, typename DataType >
PairingHeap<KeyType, DataType>::PairingHeap( const int max_nb_elt )
  : m_nb_elt( 0 )
{
  m_nodes = new PairingNode<KeyType, DataType>[ max_nb_elt ] ;

  for( int i = 0 ; i < max_nb_elt ; ++i )
  {
    m_avail_nodes.push( &m_nodes[ i ] );
  }

  m_root = nullptr ;
}


/**
* @brief Dtor
*/
template< typename KeyType, typename DataType >
PairingHeap<KeyType, DataType>::~PairingHeap( void )
{
  delete[] m_nodes ;
}


/**
* @brief Get key associated with a node
* @param elt Node to query
* @return key associated to the node
*/
template< typename KeyType, typename DataType >
KeyType PairingHeap<KeyType, DataType>::GetKey( PairingNode<KeyType, DataType> * elt )
{
  return elt->m_key ;
}


/**
* @brief Get data associated with a node
* @param elt Node to query
* @return Data associated to the node
*/
template< typename KeyType, typename DataType >
DataType PairingHeap<KeyType, DataType>::GetData( PairingNode<KeyType, DataType> * elt )
{
  return elt->m_value ;
}


/**
* @brief Insert an element inside the heap
* @param key Key of the new element
* @param data Data of the new element
* @return a pointer to The newly created element
*/
template< typename KeyType, typename DataType >
PairingNode<KeyType, DataType> * PairingHeap<KeyType, DataType>::Insert( const KeyType key , const DataType data )
{
  PairingNode<KeyType, DataType> * res = m_avail_nodes.top() ;
  m_avail_nodes.pop() ;

  // Create node
  res->m_key = key ;
  res->m_value = data ;
  res->m_child = nullptr ;
  res->m_prev = nullptr ;
  res->m_next = nullptr ;

  // Insert element in the tree
  m_root = Merge( m_root , res ) ;

  m_nb_elt++;

  return res ;
}


/**
* @brief Get element of minimum key value
* @return Pointer to the heap element with minimum value
* @note If heap is empty, return nullptr
*/
template< typename KeyType, typename DataType >
PairingNode<KeyType, DataType> * PairingHeap<KeyType, DataType>::FindMin( )
{
  return m_root ;
}

/**
 * @brief Delete element with minimum key value
 * @note If heap is empty, do nothing
 */
template< typename KeyType, typename DataType >
void PairingHeap<KeyType, DataType>::DeleteMin( void )
{
  Delete( m_root ) ;
}

/**
* @brief Get the number element in the heap
* @return number of element in the heap
* @note This is the currently active element in the heap, not the maximum size
*/
template< typename KeyType, typename DataType >
unsigned long PairingHeap<KeyType, DataType>::Size( void )
{
  return m_nb_elt ;
}


/**
* @brief Decrease key value of a specified element
* @param elt Element to decrease
* @param key the new key value
*/
template< typename KeyType, typename DataType >
void PairingHeap<KeyType, DataType>::DecreaseKey( PairingNode<KeyType, DataType> * elt , const KeyType key )
{
  // 0 -> Update key
  elt->m_key = key ;

  // 1 -> now update the heap
  if( elt != m_root )
  {
    // Disconnect the node from the heap
    if( elt->m_prev->m_child == elt ) // The node is the first of child of its parent
    {
      elt->m_prev->m_child = elt->m_next ;
    }
    else // The node is in a list
    {
      elt->m_prev->m_next = elt->m_next ;
    }

    // Update the next element (if exists )
    if( elt->m_next )
    {
      elt->m_next->m_prev = elt->m_prev ;
    }

    // Update the heap by merging the element and the root
    m_root = Merge( m_root , elt ) ;
  }

}


/**
* @brief Remove an element from the heap
* @param elt Element to remove
* @note After removal, pointed value has no meaning
*/
template< typename KeyType, typename DataType >
void PairingHeap<KeyType, DataType>::Delete( PairingNode<KeyType, DataType> * elt )
{
  // Do we remove the minimum element
  if( elt == m_root )
  {
    // Just merge the children of the root to form a heap
    m_root = TwoPassMerging( m_root->m_child ) ;
  }
  else
  {
    // Is it the first of the child of it's parents
    if( elt->m_prev->m_child == elt )
    {
      // Update parent child to take the next child
      elt->m_prev->m_child = elt->m_next ;
    }
    else // Not a child (ie: we've got preceding elements)
    {
      // Update previous element to remove elt from the list
      elt->m_prev->m_next = elt->m_next  ;
    }

    // Update previous of the next (if exists)
    if( elt->m_next )
    {
      elt->m_next->m_prev = elt->m_prev ;
    }

    // We've removed the element, but we did not ensure it's still a heap
    // -> Merge the heap with the children of the element we removed
    m_root = Merge( m_root , TwoPassMerging( elt->m_child ) ) ;
  }

  m_avail_nodes.push( elt ) ;
  --m_nb_elt ;
}


/**
* @brief Tests if the heap is empty
* @retval true if the heap is empty
* @retval false if the heap contains at least one element
*/
template< typename KeyType, typename DataType >
bool PairingHeap<KeyType, DataType>::Empty( void ) const
{
  return m_nb_elt == 0 ;
}

}
}

#endif