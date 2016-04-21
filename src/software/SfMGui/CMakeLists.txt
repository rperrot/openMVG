
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

FIND_PACKAGE( Qt5Widgets REQUIRED )

set( SFMGUI_SOURCES ImageTab.cpp MainWindow.cpp ResultTab.cpp SettingTab.cpp SfMProject.cpp SfMSettings.cpp )
set( SFMGUI_HEADERS ImageTab.hpp MainWindow.hpp ResultTab.hpp SettingTab.hpp SfMProject.hpp SfMSettings.hpp )

set( SFM_HELPERS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../SfM/SfMInitImageListingHelper.cpp )
set( SFM_HELPERS_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../SfM/SfMInitImageListingHelper.hpp )

add_executable( SfMGui WIN32 main.cpp ${SFMGUI_SOURCES} ${SFMGUI_HEADERS} ${SFM_HELPERS_SOURCES} ${SFM_HELPERS_HEADERS} )
target_link_libraries( SfMGui Qt5::Widgets stlplus openMVG_sfm openMVG_image easyexif )

# copy ressources folder into destination folder 
add_custom_command( TARGET SfMGui 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/ressources ${EXECUTABLE_OUTPUT_PATH}/ressources )

# copy sensor_width_database file to ressource folder 
add_custom_command( TARGET SfMGui 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/openMVG/exif/sensor_width_database/sensor_width_camera_database.txt ${EXECUTABLE_OUTPUT_PATH}/ressources/sensor_width_camera_database.txt ) 