#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_HIERARCHIES_LINEAR_HIERARCHY_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_HIERARCHIES_LINEAR_HIERARCHY_HH_

#include "SceneHierarchy.hh"

#include <vector>

namespace openMVG_gui
{

class SceneManager;

/**
* @brief class holding a linear hierarchy (ie: a soup of object without structure)
*/
class LinearHierarchy : public SceneHierarchy
{
  public:


    /**
    * @brief add an object to the hierarchy
    */
    void addObject( std::shared_ptr<RenderableObject> obj ) override ;

    /**
    * @brief remove an object from the hierarchy
    * @note if object does not exists in the hierarchy, do nothing
    */
    void removeObject( std::shared_ptr<RenderableObject> obj ) override ;

    /**
    * @brief Build internal structure
    */
    void prepare( void ) override ;

    /**
    * @brief Render current scene
    * @param scn Scene
    * @param ratio Aspect ratio
    */
    void render( std::shared_ptr<SceneManager> scn , const double w , const double h ) override ;

    /**
    * @brief destroy all openGL data (if any present)
    */
    void destroyGLData( void ) override ;


  private:

    std::vector < std::shared_ptr<RenderableObject> > m_objects ;

} ;

} // namespace openMVG_gui

#endif