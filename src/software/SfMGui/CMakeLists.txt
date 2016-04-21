
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

FIND_PACKAGE( Qt5Widgets REQUIRED )

set( SFMGUI_SOURCES ImageTab.cpp MainWindow.cpp ResultTab.cpp SettingTab.cpp SfMSettings.cpp )
set( SFMGUI_HEADERS ImageTab.hpp MainWindow.hpp ResultTab.hpp SettingTab.hpp SfMSettings.hpp )

add_executable( SfMGui WIN32 main.cpp ${SFMGUI_SOURCES} ${SFMGUI_HEADERS} )
target_link_libraries( SfMGui Qt5::Widgets )

# copy ressources folder into destination folder 
add_custom_command( TARGET SfMGui 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/ressources ${EXECUTABLE_OUTPUT_PATH}/ressources )
