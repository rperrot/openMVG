#ifndef _OPENMVG_GRAPH_GRAPH_UNION_FIND_HPP_
#define _OPENMVG_GRAPH_GRAPH_UNION_FIND_HPP_

#include <vector>
#include <map>

namespace openMVG
{
namespace graph
{

template< typename T >
struct UnionFindNode
{
  UnionFindNode( const T & data )
    : m_parent( nullptr ) ,
      m_data( data ) ,
      m_rank( 0 )
  {

  }

  UnionFindNode( UnionFindNode<T> & src )
    : m_parent( src.m_parent ) ,
      m_data( src.m_data ) ,
      m_rank( src.m_rank )
  {

  }

  UnionFindNode<T> operator=( const UnionFindNode<T> & src )
  {
    if( this != &src )
    {
      m_parent = src.m_parent ;
      m_data = src.m_data ;
      m_rank = src.m_rank ;
    }
    return *this ;
  }

  UnionFindNode * m_parent ;
  T m_data ;
  int m_rank ;
} ;

template< typename T >
struct TUnionFind
{
    // Make a union find set from a list of elements
    TUnionFind( const std::vector< T > & src ) ;

    // Destroy a set
    ~TUnionFind( void ) ;

    // Find parent of a node
    T Find( T & data ) ;

    // Make union of two nodes
    void Union( T & , T & ) ;

  private:

    UnionFindNode<T> * FindInternal( UnionFindNode<T> * n ) ;

    std::map< T , UnionFindNode<T> * > m_map ;

    UnionFindNode<T> * m_data ;
};


// Make a union find set from a list of elements
template< typename T>
TUnionFind<T>::TUnionFind( const std::vector< T > & src )
{
  m_data = ( UnionFindNode<T>* ) malloc( src.size() * sizeof( UnionFindNode<T> ) ) ;
  for( size_t i = 0 ; i < src.size() ; ++i )
  {
    m_data[ i ] = UnionFindNode<T>( src[i] ) ;
    m_data[ i ].m_parent = &m_data[i];
    m_map[ src[i] ] = &m_data[ i ] ;
  }
}

// Destroy a set
template< typename T>
TUnionFind<T>::~TUnionFind( void )
{
  free( m_data ) ;
  m_map.clear() ;
}

// Find parent of a node
template< typename T>
T TUnionFind<T>::Find( T & data )
{
  UnionFindNode<T> * x = m_map[ data ] ;

  // Path compression
  UnionFindNode<T> * p = FindInternal( x ) ;

  return p->m_data ;
}

// Make union of two nodes
template< typename T>
void TUnionFind<T>::Union( T & n1 , T & n2 )
{
  UnionFindNode<T> * x = m_map[ n1 ] ;
  UnionFindNode<T> * y = m_map[ n2 ] ;

  UnionFindNode<T> * xRoot = FindInternal( x ) ;
  UnionFindNode<T> * yRoot = FindInternal( y ) ;

  if( xRoot == yRoot )
  {
    return ;
  }

  // Union by rank
  if ( xRoot->m_rank < yRoot->m_rank )
  {
    xRoot->m_parent = yRoot ;
  }
  else if ( xRoot->m_rank > yRoot->m_rank )
  {
    yRoot->m_parent = xRoot ;
  }
  else
  {
    // Add y to x and increase rank of x
    yRoot->m_parent = xRoot ;
    xRoot->m_rank = xRoot->m_rank + 1 ;
  }

}

template< typename T>
UnionFindNode<T> * TUnionFind<T>::FindInternal( UnionFindNode<T> * x )
{
  if( x->m_parent != x )
  {
    x->m_parent = FindInternal( x->m_parent ) ;
  }
  return x->m_parent ;
}




}
}

#endif