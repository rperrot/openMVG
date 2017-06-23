#include "SceneManager.hh"

namespace openMVG_gui
{

/**
* @brief Ctr
* @param cam Camera
* @param s_hier Scene hierarchy manager
*/
SceneManager::SceneManager( std::shared_ptr<Camera> cam , std::shared_ptr<SceneHierarchy> s_hier )
  : m_camera( cam ) ,
    m_hierarchy( s_hier )
{

}

/**
* @brief add object to scene
*/
void SceneManager::addObject( std::shared_ptr<RenderableObject> obj )
{
  m_hierarchy->addObject( obj ) ;
}

/**
* @brief remove object from scene
*/
void SceneManager::removeObject( std::shared_ptr<RenderableObject> obj )
{
  m_hierarchy->removeObject( obj ) ;
}


/**
* @brief Get camera
* @return current camera
*/
std::shared_ptr<Camera> SceneManager::camera( void ) const
{
  return m_camera ;
}

/**
* @brief Set camera
* @param cam New camera
*/
void SceneManager::setCamera( std::shared_ptr<Camera> cam )
{
  m_camera = cam ;
}

/**
* @brief prepare scene before rendering
*/
void SceneManager::prepare( void )
{
  m_hierarchy->prepare() ;
}

/**
* @brief render current scene
*/
void SceneManager::render( const double w , const double h )
{
  m_hierarchy->render( shared_from_this() , w , h ) ;
}

// Set cameras gizmos
void SceneManager::setCameraGizmos( const std::map< int , std::shared_ptr<RenderableObject> > & objs )
{
  m_camera_gizmos = objs ;

  for( auto & it : m_camera_gizmos )
  {
    m_hierarchy->addObject( it.second ) ;
  }
}

// Remove all camera gizmos
void SceneManager::removeCameraGizmos( void )
{
  for( auto & it : m_camera_gizmos )
  {
    m_hierarchy->removeObject( it.second ) ;
  }
  m_camera_gizmos.clear() ;
}

// Get a gizmo
std::shared_ptr<RenderableObject> SceneManager::cameraGizmo( const int id ) const
{
  if( m_camera_gizmos.count( id ) > 0 )
  {
    return m_camera_gizmos.at( id ) ;
  }

  return nullptr ;
}

// Get list of camera gizmos
std::vector< std::shared_ptr<RenderableObject> > SceneManager::cameraGizmos( void ) const
{
  std::vector< std::shared_ptr<RenderableObject>> res ;
  for( auto & it : m_camera_gizmos )
  {
    res.emplace_back( it.second ) ;
  }
  return res ;
}

/**
* @brief destroy all openGL data (if any present)
*/
void SceneManager::destroyGLData( void )
{
  m_hierarchy->destroyGLData() ;
}


} // namespace openMVG_gui