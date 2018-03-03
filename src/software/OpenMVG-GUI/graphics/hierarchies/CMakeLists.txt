set( OPENMVG_GUI_GRAPHICS_HIERARCHIES_HEADERS LinearHierarchy.hh )
set( OPENMVG_GUI_GRAPHICS_HIERARCHIES_SOURCES LinearHierarchy.cc )

add_library( OpenMVG_gui_graphics_hierarchies STATIC ${OPENMVG_GUI_GRAPHICS_HIERARCHIES_HEADERS} ${OPENMVG_GUI_GRAPHICS_HIERARCHIES_SOURCES} )
target_link_libraries( OpenMVG_gui_graphics_hierarchies ${OPENGL_LIBRARIES} 
                                                        Qt5::OpenGL openMVG_numeric )
target_include_directories( OpenMVG_gui_graphics_hierarchies PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ )
target_include_directories( OpenMVG_gui_graphics_hierarchies PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../ )

set_target_properties( OpenMVG_gui_graphics_hierarchies PROPERTIES CXX_STANDARD 11 )
set_property(TARGET OpenMVG_gui_graphics_hierarchies PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)
