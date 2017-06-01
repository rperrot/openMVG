add_subdirectory( hierarchies ) 
add_subdirectory( objects )

set( OPENMVG_GUI_GRAPHICS_HEADERS Camera.hh 
                                  RenderableObject.hh 
                                  SelectableObject.hh 
                                  SceneHierarchy.hh 
                                  SceneManager.hh 
                                  Shader.hh 
                                  ShaderProgram.hh )
set( OPENMVG_GUI_GRAPHICS_SOURCES Camera.cc 
                                  RenderableObject.cc 
                                  SceneHierarchy.cc 
                                  SceneManager.cc 
                                  SelectableObject.cc 
                                  Shader.cc 
                                  ShaderProgram.cc )

add_library( OpenMVG_gui_graphics STATIC ${OPENMVG_GUI_GRAPHICS_HEADERS} ${OPENMVG_GUI_GRAPHICS_SOURCES} )
target_link_libraries( OpenMVG_gui_graphics OpenMVG_gui_utils 
                                            OpenMVG_gui_graphics_hierarchies 
                                            OpenMVG_gui_graphics_objects
                                            ${OPENGL_LIBRARIES} 
                                            Qt5::OpenGL )
set_target_properties( OpenMVG_gui_graphics PROPERTIES CXX_STANDARD 11 )


target_include_directories( OpenMVG_gui_graphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../utils )
target_include_directories( OpenMVG_gui_graphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/hierarchies )
target_include_directories( OpenMVG_gui_graphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/objects )
