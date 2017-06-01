#ifndef _SCENE_HIERARCHY_HH_
#define _SCENE_HIERARCHY_HH_

#include "RenderableObject.hh"

namespace openMVG_gui
{

class SceneManager ;

/**
* @brief Class managing a hierarchy
* this contains renderable objects and is responsible for rendering it
* -> Maybe used for culling
*/
class SceneHierarchy
{
  public:

    /**
    * @brief Ctr
    */
    SceneHierarchy( void ) ;

    /**
    * @brief add an object to the hierarchy
    */
    virtual void addObject( std::shared_ptr<RenderableObject> obj ) = 0 ;

    /**
    * @brief remove an object from the hierarchy 
    * @note if object does not exists in the hierarchy, do nothing 
    */
    virtual void removeObject( std::shared_ptr<RenderableObject> obj ) = 0 ; 

    /**
    * @brief Build internal structure
    */
    virtual void prepare( void ) = 0 ;

    /**
    * @brief Render current scene
    * @param mgr Scene manager (containing camera and lights)
    */
    virtual void render( std::shared_ptr<SceneManager> scn , const double aspect_ratio ) = 0 ;

  private:


} ;

} // namespace openMVG_gui

#endif