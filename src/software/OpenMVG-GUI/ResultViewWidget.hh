#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_VIEW_WIDGET_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_VIEW_WIDGET_HH_

#include "SceneManager.hh"

#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QWidget>

namespace openMVG_gui
{
/**
* @brief Widget used to display a 3d view of the result
*/
class ResultViewWidget : public QOpenGLWidget
{
  public:

    /**
    * @brief Ctr
    * @param parent Parent widget
    */
    ResultViewWidget( QWidget * parent = nullptr ) ;

    ~ResultViewWidget( void ) ; 

    /**
    * @brief Initialize openGL context
    */
    void initializeGL( void ) override ;

    /**
    * @brief main render loop
    */
    void paintGL( void ) override ;

    /**
    * @brief When image is resized
    */
    void resizeGL( int w, int h ) override ;

    /**
    * @brief Set current scene
    */
    void setScene( std::shared_ptr<SceneManager> mgr ) ;

    /**
    * @brief get standard point shader
    */
    std::shared_ptr<ShaderProgram> pointShader( void ) const ;

    /**
    * @brief get standard grid
    */
    std::shared_ptr<RenderableObject> grid( void ) const ;

    /**
    * @brief get standard spherical gizmo
    */
    std::shared_ptr<RenderableObject> sphericalGizmo( void ) const ;

    /**
    * @brief Prepare objects before rendering
    * This must be called each time objects have been added to the scene manager
    */
    void prepareObjects( void ) ;

    /**
    * @brief Update track size based on screen
    */
    void updateTrackballSize( void ) ;

    /**
    * @brief Update scene by zooming in or out
    */
    void wheelEvent( QWheelEvent * event ) override ;

    /**
    * @brief Mouse event management
    * Used to pan/rotate camera for ex
    */
    void mousePressEvent( QMouseEvent * event ) override ;

    /**
    * @brief Mouse event management
    * Used to pan/rotate camera for ex
    */
    void mouseMoveEvent( QMouseEvent * event ) override ;

    /**
    * @brief Mouse event management
    * Use to pan/rotate camera for ex
    */
    void mouseReleaseEvent( QMouseEvent * event ) override ;

  private slots : 

    /**
    * @brief clean openGL data before quitting 
    */
    void destroyGLData( void ) ; 

  private:

    

    void makeConnections( void ) ;

    std::shared_ptr<SceneManager> m_scn ;
    std::shared_ptr<ShaderProgram> m_point_shader ;
    std::shared_ptr<RenderableObject> m_grid ;
    std::shared_ptr<RenderableObject> m_sph_gizmo ;

    // For mouse motion
    double m_last_mouse_x ;
    double m_last_mouse_y ;

    Q_OBJECT 
} ;

} // namespace openMVG_gui

#endif