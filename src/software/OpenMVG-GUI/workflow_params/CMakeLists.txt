set( WORKFLOW_PARAMS_HEADERS FeatureParams.hh 
                             GlobalSfMParams.hh 
                             IncrementalSfMParams.hh 
                             IntrinsicParams.hh 
                             IntrinsicRefinerOptions.hh 
                             MatchingParams.hh )
set( WORKFLOW_PARAMS_SOURCES FeatureParams.cc
                             GlobalSfMParams.cc 
                             IncrementalSfMParams.cc 
                             IntrinsicParams.cc 
                             IntrinsicRefinerOptions.cc 
                             MatchingParams.cc )

add_library( OpenMVG_gui_workflow_params STATIC ${WORKFLOW_PARAMS_HEADERS} ${WORKFLOW_PARAMS_SOURCES} ) 

set_target_properties( OpenMVG_gui_workflow_params PROPERTIES CXX_STANDARD 11 )

target_link_libraries( OpenMVG_gui_workflow_params openMVG_features )

add_subdirectory( widgets )