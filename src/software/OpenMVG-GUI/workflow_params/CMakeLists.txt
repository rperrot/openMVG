set( WORKFLOW_PARAMS_HEADERS FeatureParams.hh 
                             GlobalSfMParams.hh 
                             IncrementalSfMParams.hh 
                             IntrinsicParams.hh 
                             IntrinsicRefinerOptions.hh 
                             MatchingParams.hh 
                             SfMMethod.hh )
set( WORKFLOW_PARAMS_SOURCES FeatureParams.cc
                             GlobalSfMParams.cc 
                             IncrementalSfMParams.cc 
                             IntrinsicParams.cc 
                             IntrinsicRefinerOptions.cc 
                             MatchingParams.cc )

add_library( OpenMVG_gui_workflow_params STATIC ${WORKFLOW_PARAMS_HEADERS} ${WORKFLOW_PARAMS_SOURCES} ) 

set_target_properties( OpenMVG_gui_workflow_params PROPERTIES CXX_STANDARD 11 )

target_link_libraries( OpenMVG_gui_workflow_params openMVG_camera openMVG_features openMVG_sfm )

set_property(TARGET OpenMVG_gui_workflow_params PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)

add_subdirectory( widgets )