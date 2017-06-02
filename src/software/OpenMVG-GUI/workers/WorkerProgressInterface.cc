#include "WorkerProgressInterface.hh"

namespace openMVG_gui
{

WorkerProgressInterface::WorkerProgressInterface( void )
  : m_canceled( false )
{

}

/** @brief Indicator if the current operation should be aborted.
* @return Return true if the process has been canceled by the user.
**/
bool WorkerProgressInterface::hasBeenCanceled() const
{
  return m_canceled ;
}

unsigned long WorkerProgressInterface::operator+= ( unsigned long ulIncrement )
{
  std::lock_guard<std::mutex> lock( m_mutex );

  unsigned long res = C_Progress::operator+=( ulIncrement ) ;
  emit increment( ulIncrement ) ;
  return res ;
}

unsigned long WorkerProgressInterface::operator++()
{
  std::lock_guard<std::mutex> lock( m_mutex );

  unsigned long res = C_Progress::operator+= ( 1 ) ;
  emit increment( 1 ) ;
  return res ;
}

void WorkerProgressInterface::cancel( void )
{
  m_canceled = true ;
}

}