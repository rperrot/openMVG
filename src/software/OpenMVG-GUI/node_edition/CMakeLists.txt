set( NODE_EDITION_HDRS NodeItem.hh 
                       NodeStyle.hh 
                       NodeView.hh 
                       PipelineEditor.hh )
set( NODE_EDITION_SRCS NodeItem.cc 
                       NodeStyle.cc
                       NodeView.cc 
                       PipelineEditor.cc )

add_library( OpenMVG_gui_node_edition ${NODE_EDITION_HDRS} ${NODE_EDITION_SRCS} )
target_link_libraries( OpenMVG_gui_node_edition Qt5::Widgets )

target_include_directories( OpenMVG_gui_node_edition PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ )

set_target_properties( OpenMVG_gui_node_edition PROPERTIES CXX_STANDARD 11 )
set_property(TARGET OpenMVG_gui_node_edition PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)
