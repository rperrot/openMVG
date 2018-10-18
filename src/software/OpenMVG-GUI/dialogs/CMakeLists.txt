set( OPENMVG_GUI_DIALOGS_HEADERS AddSensorEntryDialog.hh 
                                 ApplicationSettingsDialog.hh
                                 AutomaticReconstructionDialog.hh 
                                 ClusterComputationSettingsDialog.hh
                                 ColorIndicatorWidget.hh
                                 DoubleProgressBarDialog.hh
                                 FeaturesViewerWidget.hh
                                 MaskDefinitionDialog.hh
                                 MaskView.hh
                                 NewProjectDialog.hh 
                                 SensorDatabaseDialog.hh
                                 SensorDatabaseModel.hh
                                 ZoomableGraphicsView.hh )

set( OPENMVG_GUI_DIALOGS_SOURCES AddSensorEntryDialog.cc
                                 ApplicationSettingsDialog.cc
                                 AutomaticReconstructionDialog.cc  
                                 ClusterComputationSettingsDialog.cc
                                 ColorIndicatorWidget.cc 
                                 DoubleProgressBarDialog.cc
                                 FeaturesViewerWidget.cc
                                 MaskDefinitionDialog.cc
                                 MaskView.cc 
                                 NewProjectDialog.cc
                                 SensorDatabaseDialog.cc
                                 SensorDatabaseModel.cc
                                 ZoomableGraphicsView.cc )


add_library( OpenMVG_gui_dialogs STATIC ${OPENMVG_GUI_DIALOGS_HEADERS} ${OPENMVG_GUI_DIALOGS_SOURCES} )
target_link_libraries( OpenMVG_gui_dialogs OpenMVG_gui_utils Qt5::Widgets openMVG_stlplus openMVG_numeric )
set_target_properties( OpenMVG_gui_dialogs PROPERTIES CXX_STANDARD 11 )

target_include_directories( OpenMVG_gui_dialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../ )
set_property(TARGET OpenMVG_gui_dialogs PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)