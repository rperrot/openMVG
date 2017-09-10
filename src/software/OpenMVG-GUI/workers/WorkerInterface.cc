#include "WorkerInterface.hh"

namespace openMVG_gui
{

/**
 * @brief next action to be executed after this worker
 */
WorkerInterface::WorkerInterface( const WorkerNextAction &na )
  : m_next_action( na )
{
}

/**
 * @brief remaining actions to be executed after this worker thread
 */
WorkerNextAction WorkerInterface::nextAction( void ) const
{
  return m_next_action;
}

} // namespace openMVG_gui