set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set( OPENMVG_GUI_WORKERS_HEADERS WorkerColorComputation.hh 
                                 WorkerFeaturesComputation.hh 
                                 WorkerFeaturesProviderLoad.hh 
                                 WorkerGeometricFiltering.hh 
                                 WorkerGlobalSfMComputation.hh 
                                 WorkerIncrementalSfMComputation.hh 
                                 WorkerMatchesComputation.hh
                                 WorkerMatchesProviderLoad.hh 
                                 WorkerProgressInterface.hh 
                                 WorkerProjectCreation.hh 
                                 WorkerRegionsProviderLoad.hh
                                 WorkerThumbnailGeneration.hh 

                                 WorkerInterface.hh 
                                 WorkerNextAction.hh ) 
set( OPENMVG_GUI_WORKERS_SOURCES WorkerColorComputation.cc 
                                 WorkerFeaturesComputation.cc 
                                 WorkerFeaturesProviderLoad.cc 
                                 WorkerGeometricFiltering.cc
                                 WorkerGlobalSfMComputation.cc 
                                 WorkerIncrementalSfMComputation.cc 
                                 WorkerMatchesComputation.cc 
                                 WorkerMatchesProviderLoad.cc 
                                 WorkerProgressInterface.cc 
                                 WorkerProjectCreation.cc 
                                 WorkerRegionsProviderLoad.cc 
                                 WorkerThumbnailGeneration.cc 

                                 WorkerInterface.cc 
                                 WorkerNextAction.cc )

add_library( OpenMVG_gui_workers STATIC ${OPENMVG_GUI_WORKERS_HEADERS} ${OPENMVG_GUI_WORKERS_SOURCES} )
target_link_libraries( OpenMVG_gui_workers Qt5::Widgets Qt5::OpenGL )
target_include_directories( OpenMVG_gui_workers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)

set_target_properties( OpenMVG_gui_workers PROPERTIES CXX_STANDARD 11 )
