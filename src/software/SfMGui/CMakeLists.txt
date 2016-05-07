
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

FIND_PACKAGE( Qt5Widgets REQUIRED )

set( SFMGUI_SOURCES ImageTab.cpp MainWindow.cpp ProjectCreationWizard.cpp ProjectCreationWizardInputPage.cpp ProjectCreationWizardOutputPage.cpp ProjectCreatorThread.cpp ResultTab.cpp SettingTab.cpp SfMProject.cpp SfMSettings.cpp )
set( SFMGUI_HEADERS ImageTab.hpp MainWindow.hpp ProjectCreationWizard.hpp ProjectCreationWizardInputPage.hpp ProjectCreationWizardOutputPage.hpp ProjectCreatorThread.hpp ResultTab.hpp SettingTab.hpp SfMProject.hpp SfMSettings.hpp )

set( SFMGUI_DEPS_SOURCES dependencies/waitingspinnerwidget.cpp )
set( SFMGUI_DEPS_HEADERS dependencies/waitingspinnerwidget.h )

set( SFM_HELPERS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../SfM/SfMInitImageListingHelper.cpp )
set( SFM_HELPERS_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../SfM/SfMInitImageListingHelper.hpp )
                    
IF( APPLE )
  
  add_executable( SfMGui MACOSX_BUNDLE main.cpp ${SFMGUI_SOURCES} ${SFMGUI_HEADERS} ${SFM_HELPERS_SOURCES} ${SFM_HELPERS_HEADERS} ${SFMGUI_DEPS_SOURCES} ${SFMGUI_DEPS_HEADERS} )
  target_link_libraries( SfMGui Qt5::Widgets stlplus openMVG_sfm openMVG_image easyexif )

  # copy resources folder into destination folder 
  add_custom_command( TARGET SfMGui 
                      POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${EXECUTABLE_OUTPUT_PATH}/SfMGui.app/Contents/MacOS/resources )

  # copy sensor_width_database file to resource folder 
  add_custom_command( TARGET SfMGui 
                      POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/openMVG/exif/sensor_width_database/sensor_width_camera_database.txt ${EXECUTABLE_OUTPUT_PATH}/SfMGui.app/Contents/MacOS/resources/sensor_width_camera_database.txt ) 

  set_target_properties(SfMGui PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/PlatformSpecificResources/Info.plist.in)


ELSE( APPLE )

  add_executable( SfMGui WIN32 main.cpp ${SFMGUI_SOURCES} ${SFMGUI_HEADERS} ${SFM_HELPERS_SOURCES} ${SFM_HELPERS_HEADERS} ${SFMGUI_DEPS_SOURCES} ${SFMGUI_DEPS_HEADERS} )
  target_link_libraries( SfMGui Qt5::Widgets stlplus openMVG_sfm openMVG_image easyexif )

  # copy resources folder into destination folder 
  add_custom_command( TARGET SfMGui 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${EXECUTABLE_OUTPUT_PATH}/resources )

  # copy sensor_width_database file to resource folder 
  add_custom_command( TARGET SfMGui 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/openMVG/exif/sensor_width_database/sensor_width_camera_database.txt ${EXECUTABLE_OUTPUT_PATH}/resources/sensor_width_camera_database.txt )   
ENDIF( APPLE )