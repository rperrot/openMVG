set( OPENMVG_GRAPHICS_OBJECTS_HEADERS CameraGizmo.hh 
                                      Grid.hh 
                                      PointCloud.hh
                                      SphericalGizmo.hh  )
set( OPENMVG_GRAPHICS_OBJECTS_SOURCES CameraGizmo.cc 
                                      Grid.cc 
                                      PointCloud.cc
                                      SphericalGizmo.cc ) 

add_library( OpenMVG_gui_graphics_objects STATIC ${OPENMVG_GRAPHICS_OBJECTS_HEADERS} ${OPENMVG_GRAPHICS_OBJECTS_SOURCES} )
target_link_libraries( OpenMVG_gui_graphics_objects ${OPENGL_LIBRARIES} Qt5::OpenGL openMVG_sfm )
target_include_directories( OpenMVG_gui_graphics_objects PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ )
target_include_directories( OpenMVG_gui_graphics_objects PRIVATE ${OPENGL_INCLUDE_DIR} )

set_target_properties( OpenMVG_gui_graphics_objects PROPERTIES CXX_STANDARD 11 )
set_property(TARGET OpenMVG_gui_graphics_objects PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)