set( OPENMVG_GUI_DIALOGS_HEADERS ApplicationSettingsDialog.hh
                                 AutomaticReconstructionDialog.hh 
                                 ClusterComputationSettingsDialog.hh
                                 ColorIndicatorWidget.hh
                                 DoubleProgressBarDialog.hh
                                 FeaturesViewerWidget.hh
                                 MaskDefinitionDialog.hh
                                 MaskView.hh
                                 NewProjectDialog.hh 
                                 ZoomableGraphicsView.hh )

set( OPENMVG_GUI_DIALOGS_SOURCES ApplicationSettingsDialog.cc
                                 AutomaticReconstructionDialog.cc  
                                 ClusterComputationSettingsDialog.cc
                                 ColorIndicatorWidget.cc 
                                 DoubleProgressBarDialog.cc
                                 FeaturesViewerWidget.cc
                                 MaskDefinitionDialog.cc
                                 MaskView.cc 
                                 NewProjectDialog.cc
                                 ZoomableGraphicsView.cc )


add_library( OpenMVG_gui_dialogs STATIC ${OPENMVG_GUI_DIALOGS_HEADERS} ${OPENMVG_GUI_DIALOGS_SOURCES} )
target_link_libraries( OpenMVG_gui_dialogs Qt5::Widgets stlplus openMVG_numeric )
set_target_properties( OpenMVG_gui_dialogs PROPERTIES CXX_STANDARD 11 )

target_include_directories( OpenMVG_gui_dialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../ )
set_property(TARGET OpenMVG_gui_dialogs PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)