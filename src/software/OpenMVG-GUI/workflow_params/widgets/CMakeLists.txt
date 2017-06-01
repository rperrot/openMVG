set( WORKFLOW_PARAMS_WIDGETS_HEADERS FeatureParamsDialog.hh 
                                     MatchingParamsDialog.hh 
                                     SfMParamsDialog.hh )
set( WORKFLOW_PARAMS_WIDGETS_SOURCES FeatureParamsDialog.cc 
                                     MatchingParamsDialog.cc 
                                     SfMParamsDialog.cc )

add_library( OpenMVG_gui_workflow_params_widgets STATIC 
             ${WORKFLOW_PARAMS_WIDGETS_HEADERS} 
             ${WORKFLOW_PARAMS_WIDGETS_SOURCES} )
target_link_libraries( OpenMVG_gui_workflow_params_widgets Qt5::Widgets Qt5::OpenGL )
target_include_directories( OpenMVG_gui_workflow_params_widgets PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../ )

set_target_properties( OpenMVG_gui_workflow_params_widgets PROPERTIES CXX_STANDARD 11 )
