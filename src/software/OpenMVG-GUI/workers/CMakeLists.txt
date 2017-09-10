set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set( OPENMVG_GUI_WORKERS_HEADERS WorkerAutomaticReconstruction.hh
                                 WorkerClusterComputation.hh
                                 WorkerColorComputation.hh 
                                 WorkerExportClustersToMVS.hh
                                 WorkerExportToMVE.hh
                                 WorkerExportToOpenMVS.hh
                                 WorkerExportToPMVS.hh 
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
                                 WorkerSfMDataLoad.hh
                                 WorkerThumbnailGeneration.hh 
                                 WorkerInterface.hh 
                                 WorkerNextAction.hh ) 
                                 
set( OPENMVG_GUI_WORKERS_SOURCES WorkerAutomaticReconstruction.cc
                                 WorkerClusterComputation.cc
                                 WorkerColorComputation.cc 
                                 WorkerExportClustersToMVS.cc
                                 WorkerExportToMVE.cc
                                 WorkerExportToOpenMVS.cc
                                 WorkerExportToPMVS.cc
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
                                 WorkerSfMDataLoad.cc
                                 WorkerThumbnailGeneration.cc 

                                 WorkerInterface.cc 
                                 WorkerNextAction.cc )

add_library( OpenMVG_gui_workers STATIC ${OPENMVG_GUI_WORKERS_HEADERS} ${OPENMVG_GUI_WORKERS_SOURCES} )
target_link_libraries( OpenMVG_gui_workers Qt5::Widgets Qt5::OpenGL domset )
target_include_directories( OpenMVG_gui_workers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_include_directories( OpenMVG_gui_workers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../dialogs )
target_include_directories( OpenMVG_gui_workers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../graphics )

set_target_properties( OpenMVG_gui_workers PROPERTIES CXX_STANDARD 11 )
