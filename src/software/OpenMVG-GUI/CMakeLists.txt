find_package( Qt5 COMPONENTS Widgets OpenGL WebEngineWidgets Network REQUIRED )

find_package( OpenGL REQUIRED )

if( NOT Qt5Widgets_FOUND )
  message( "Qt5Widgets not found" )
else()
  message( "Qt5Widgets version : ${Qt5Widgets_VERSION}" )
endif()

if( NOT Qt5OpenGL_FOUND )
  message( "Qt5OpenGL not found" ) 
endif()

if( NOT Qt5WebEngineWidgets_FOUND )
  message( "Qt5WebEngineWidgets not found" )
endif()

if( NOT OPENGL_FOUND )
  message( "OpenGL not found" )
endif()



# Ensure Qt and openGL are present, and Qt >= 5.6 
if( Qt5Widgets_FOUND AND Qt5OpenGL_FOUND AND OPENGL_FOUND AND Qt5WebEngineWidgets_FOUND ) #AND Qt5Widgets_VERSION GREATER 5.6.0 )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory( dialogs ) 
add_subdirectory( graphics )
#add_subdirectory( node_edition )
add_subdirectory( utils )
add_subdirectory( workers ) 
add_subdirectory( workflow_params )


set( OPENMVG_GUI_HEADERS ApplicationSettings.hh 
                         ApplicationState.hh 
                         ConsoleWidget.hh
                         DetailListWidget.hh 
                         ImageListWidget.hh 
                         MainWindow.hh 
                         OpenMVGImageInterface.hh
                         Project.hh 
                         ProjectPaths.hh
                         ReconstructionSummaryWidget.hh
                         ResultViewWidget.hh 
                         Version.hh )
set( OPENMVG_GUI_SOURCES ApplicationSettings.cc 
                         ConsoleWidget.cc 
                         DetailListWidget.cc
                         ImageListWidget.cc
                         MainWindow.cc 
                         OpenMVGImageInterface.cc
                         Project.cc 
                         ProjectPaths.cc
                         ReconstructionSummaryWidget.cc
                         ResultViewWidget.cc
                         Version.cc )

if( APPLE )

  set_source_files_properties( ${CMAKE_CURRENT_SOURCE_DIR}/openmvg.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources" )
  
  add_executable( OpenMVG-GUI MACOSX_BUNDLE main.cc ${OPENMVG_GUI_HEADERS} ${OPENMVG_GUI_SOURCES} application.rc ressources/ressources.qrc openmvg.icns )
  set_target_properties(OpenMVG-GUI PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/ressources/Info.plist.in )

  set( OpenMVG_GUI_PATH "${EXECUTABLE_OUTPUT_PATH}/OpenMVG-GUI.app/Contents/MacOS/" )

  else( APPLE )
  add_executable( OpenMVG-GUI WIN32 main.cc ${OPENMVG_GUI_HEADERS} ${OPENMVG_GUI_SOURCES} application.rc ressources/ressources.qrc )
  set( OpenMVG_GUI_PATH "${EXECUTABLE_OUTPUT_PATH}" ) 
endif( APPLE )

set_target_properties( OpenMVG-GUI PROPERTIES CXX_STANDARD 11 )
set_property(TARGET OpenMVG-GUI PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)
target_link_libraries( OpenMVG-GUI 
                       # sub libraries 
                       OpenMVG_gui_dialogs
                       OpenMVG_gui_graphics
                       # OpenMVG_gui_node_edition 
                       OpenMVG_gui_utils
                       OpenMVG_gui_workers 
                       OpenMVG_gui_workflow_params
                       OpenMVG_gui_workflow_params_widgets
                       # Qt 
                       Qt5::Widgets 
                       Qt5::OpenGL 
                       Qt5::WebEngineWidgets
                       # openGL 
                       ${OPENGL_LIBRARIES}
                       # openMVG
                       openMVG_exif
                       openMVG_features 
                       openMVG_image
                       openMVG_matching_image_collection
                       openMVG_sfm
                       # third party 
                       openMVG_stlplus
                       vlsift )

target_include_directories( OpenMVG-GUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dialogs )                   
target_include_directories( OpenMVG-GUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/graphics )
target_include_directories( OpenMVG-GUI PRIVATE ${OPENGL_INCLUDE_DIR} )

add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OpenMVG_GUI_PATH}/ressources" )
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OpenMVG_GUI_PATH}/ressources/sensor_database" )
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../openMVG/exif/sensor_width_database/sensor_width_camera_database.txt" 
    "${OpenMVG_GUI_PATH}/ressources/sensor_database/sensor_width_camera_database.txt" ) 
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ressources/point_shader.frag" 
    "${OpenMVG_GUI_PATH}/ressources/point_shader.frag" ) 
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ressources/point_shader.vert" 
    "${OpenMVG_GUI_PATH}/ressources/point_shader.vert" ) 
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OpenMVG_GUI_PATH}/icons" )
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ressources/icons/plus.svg" 
    "${OpenMVG_GUI_PATH}/icons/plus.svg" ) 
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ressources/icons/minus.svg" 
    "${OpenMVG_GUI_PATH}/icons/minus.svg" ) 

# Copy the plugin into the good folder 


INSTALL( TARGETS OpenMVG-GUI DESTINATION bin )

# employ deploy Qt on executable 
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
if( APPLE )
  find_program(DEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}" )
else()
  if( WIN32 )
    find_program(DEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
  endif()
endif()

message( STATUS "Found deploy executable here : ${DEPLOYQT_EXECUTABLE}" )

if( WIN32 )
add_custom_command(TARGET OpenMVG-GUI POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${DEPLOYQT_EXECUTABLE}"
            "$<TARGET_FILE:OpenMVG-GUI>"
    COMMENT "Running windeployqt..." )
else()
  if( APPLE )
    add_custom_command(TARGET OpenMVG-GUI POST_BUILD
    COMMAND "${DEPLOYQT_EXECUTABLE}"
        "$<TARGET_FILE_DIR:OpenMVG-GUI>/../.."
        -always-overwrite
    COMMENT "Running macdeployqt..." )
  endif()
endif()

set(CPACK_PACKAGE_EXECUTABLES "${OPENMVG_BUNDLE_FILE}" "OpenMVG")

endif()
