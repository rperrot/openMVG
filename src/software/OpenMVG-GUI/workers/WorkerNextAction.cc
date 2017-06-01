#include "WorkerNextAction.hh"

namespace openMVG_gui
{
/**
* @brief Bitwise operation
*/
WorkerNextAction operator|( const WorkerNextAction & a , const WorkerNextAction & b )
{
  return static_cast<WorkerNextAction>( static_cast<int>( a ) | static_cast<int>( b ) ) ;
}

/**
* @brief Indicate if act contains a set of op
* @param act Complex operation
* @param op Single operation
* @retval true if op is fully in act
* @retval false else
*/
bool contains( const WorkerNextAction & act ,
               const WorkerNextAction & op )
{
  return ( act & op ) != 0 ;
}

/**
* @brief Remove an operation from a complex set of operation
* @param act complex set of operation
* @param op Operation
* @return act without op
*/
WorkerNextAction remove( const WorkerNextAction & act , const WorkerNextAction & op )
{
  return static_cast<WorkerNextAction>( static_cast<int>( act ) & ( ~static_cast<int>( op ) ) ) ;
}

} // namespace openMVG_gui 