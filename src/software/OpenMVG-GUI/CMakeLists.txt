find_package( Qt5Widgets REQUIRED )
find_package( Qt5OpenGL REQUIRED )
find_package( OpenGL REQUIRED )

# Ensure Qt and openGL are present, and Qt >= 5.6 
if( Qt5Widgets_FOUND AND Qt5OpenGL_FOUND AND OPENGL_FOUND  AND NOT (Qt5Widgets_VERSION LESS 5.6.0))

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory( dialogs ) 
add_subdirectory( graphics )
add_subdirectory( utils )
add_subdirectory( workers ) 
add_subdirectory( workflow_params )


set( OPENMVG_GUI_HEADERS ApplicationState.hh 
                         DetailListWidget.hh 
                         ImageListWidget.hh 
                         MainWindow.hh 
                         OpenMVGImageInterface.hh
                         Project.hh 
                         ResultViewWidget.hh 
                         Version.hh )
set( OPENMVG_GUI_SOURCES DetailListWidget.cc
                         ImageListWidget.cc
                         MainWindow.cc 
                         OpenMVGImageInterface.cc
                         Project.cc 
                         ResultViewWidget.cc )

#if( APPLE )
#add_executable( OpenMVG-GUI MACOSX_BUNDLE main.cc ${OPENMVG_GUI_HEADERS} ${OPENMVG_GUI_SOURCES} )
#else( APPLE )
add_executable( OpenMVG-GUI WIN32 main.cc ${OPENMVG_GUI_HEADERS} ${OPENMVG_GUI_SOURCES} application.rc ressources/ressources.qrc )

# create ressource dirs 
#endif( APPLE )

set_target_properties( OpenMVG-GUI PROPERTIES CXX_STANDARD 11 )
set_property(TARGET OpenMVG-GUI PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)
target_link_libraries( OpenMVG-GUI 
                       # sub libraries 
                       OpenMVG_gui_dialogs
                       OpenMVG_gui_graphics
                       OpenMVG_gui_utils
                       OpenMVG_gui_workers 
                       OpenMVG_gui_workflow_params
                       OpenMVG_gui_workflow_params_widgets
                       # Qt 
                       Qt5::Widgets 
                       Qt5::OpenGL 
                       # openGL 
                       ${OPENGL_LIBRARIES}
                       # openMVG
                       openMVG_features 
                       openMVG_image
                       openMVG_matching_image_collection
                       openMVG_sfm
                       # third party 
                       easyexif
                       stlplus
                       vlsift )

target_include_directories( OpenMVG-GUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dialogs )                   
target_include_directories( OpenMVG-GUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/graphics )
target_include_directories( OpenMVG-GUI PRIVATE ${OPENGL_INCLUDE_DIR} )

add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_OUTPUT_PATH}/ressources" )
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_OUTPUT_PATH}/ressources/sensor_database" )
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../../openMVG/exif/sensor_width_database/sensor_width_camera_database.txt" 
    "${EXECUTABLE_OUTPUT_PATH}/ressources/sensor_database/sensor_width_camera_database.txt" ) 
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ressources/point_shader.frag" 
    "${EXECUTABLE_OUTPUT_PATH}/ressources/point_shader.frag" ) 
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ressources/point_shader.vert" 
    "${EXECUTABLE_OUTPUT_PATH}/ressources/point_shader.vert" ) 
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory "${EXECUTABLE_OUTPUT_PATH}/icons" )
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ressources/icons/plus.svg" 
    "${EXECUTABLE_OUTPUT_PATH}/icons/plus.svg" ) 
add_custom_command( TARGET OpenMVG-GUI POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ressources/icons/minus.svg" 
    "${EXECUTABLE_OUTPUT_PATH}/icons/minus.svg" ) 



endif()