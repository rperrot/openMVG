
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

FIND_PACKAGE( Qt5Widgets REQUIRED )

set( SFMGUI_SOURCES ImageTab.cpp MainWindow.cpp ResultTab.cpp SettingTab.cpp )
set( SFMGUI_HEADERS ImageTab.hpp MainWindow.hpp ResultTab.hpp SettingTab.hpp )

add_executable( SfMGui WIN32 main.cpp ${SFMGUI_SOURCES} ${SFMGUI_HEADERS} )
target_link_libraries( SfMGui Qt5::Widgets )