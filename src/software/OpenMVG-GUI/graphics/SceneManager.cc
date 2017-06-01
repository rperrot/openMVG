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
void SceneManager::render( const double aspect )
{
  m_hierarchy->render( shared_from_this() , aspect ) ;
}

} // namespace openMVG_gui