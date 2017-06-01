#ifndef _SCENE_MANAGER_HH_
#define _SCENE_MANAGER_HH_

#include "Camera.hh"
#include "SceneHierarchy.hh"

#include <memory>

namespace openMVG_gui
{

/**
* @brief class used to manage a render scene
* contains objects
*/
class SceneManager : public std::enable_shared_from_this<SceneManager>
{
  public:

    /**
    * @brief Ctr
    * @param cam Camera
    * @param s_hier Scene hierarchy manager
    */
    SceneManager( std::shared_ptr<Camera> cam , std::shared_ptr<SceneHierarchy> s_hier ) ;

    /**
    * @brief add object to scene
    */
    void addObject( std::shared_ptr<RenderableObject> obj ) ;

    /**
    * @brief remove object from scene 
    */
    void removeObject( std::shared_ptr<RenderableObject> obj ) ;

    /**
    * @brief Get camera
    * @return current camera
    */
    std::shared_ptr<Camera> camera( void ) const ;

    /**
    * @brief Set camera
    * @param cam New camera
    */
    void setCamera( std::shared_ptr<Camera> cam ) ;

    /**
    * @brief prepare scene before rendering
    */
    void prepare( void ) ;

    /**
    * @brief render current scene
    */
    void render( const double aspect );

  private:

    std::shared_ptr<SceneHierarchy> m_hierarchy ;
    std::shared_ptr<Camera> m_camera ;

} ;

} // namespace openMVG_gui

#endif