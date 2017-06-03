#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_RENDERABLE_OBJECT_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_RENDERABLE_OBJECT_HH_

#include "AABB.hh"
#include "ShaderProgram.hh"

namespace openMVG_gui
{

/**
* @brief Class holding an object that can be rendered in the 3d view
* @note it contains a single shader and visibility information
*/
class RenderableObject
{
  public:

    /**
    * @brief Ctr
    * @param shader Shader to use for this object
    * @param visible Indicate if object is visible
    */
    RenderableObject( std::shared_ptr<ShaderProgram> shader ,
                      const bool visible = true ) ;

    /**
    * @brief Get model matrix
    * @return current model matrix (in left hand system)
    */
    virtual openMVG::Mat4 modelMat( void ) const ;

    /**
    * @brief Prepare object before rendering (ie: create buffers, prepare data)
    */
    virtual void prepare( void ) = 0 ;

    /**
    * @brief Draw code for the object
    */
    virtual void draw( void ) const = 0 ;

    /**
    * @brief Get shader of the object
    * @return current shader
    */
    std::shared_ptr<ShaderProgram> shader( void ) const ;

    /**
    * @brief Set shader for this object
    * @param shader New shader to assign to this object
    */
    void setShader( std::shared_ptr<ShaderProgram> shader ) ;

    /**
    * @brief Indicate if object should be rendered
    * @retval true Object will be rendered
    * @retval false Object won't be rendered
    */
    bool isVisible( void ) const ;

    /**
    * @brief Set visibility of the object
    * @param vis Visibility of the object
    */
    void setVisible( const bool vis ) ;

    /**
    * @brief Switch between visible/invisble state
    */
    void switchVisibility( void ) ;

  protected:

    std::shared_ptr<ShaderProgram> m_shader ;
    bool m_prepared ;

  private:

    bool m_is_visible ;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

} ;

} // namespace openMVG_gui

#endif