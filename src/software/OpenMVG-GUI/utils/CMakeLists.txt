set( OPENMVG_GUI_UTILS_HEADERS BellBall.hh 
                               BoundingSphere.hh 
                               DualQuaternion.hh 
                               FeaturesStats.hh
                               ImageDescriberInfo.hh 
                               MatchingStats.hh
                               Plane.hh 
                               PlyLoader.hh
                               Sphere.hh 
                               StdStreamHandlers.hh
                               UtilString.hh )
set( OPENMVG_GUI_UTILS_SOURCES BellBall.cc 
                               BoundingSphere.cc 
                               DualQuaternion.cc 
                               FeaturesStats.cc
                               ImageDescriberInfo.cc
                               MatchingStats.cc
                               Plane.cc 
                               PlyLoader.cc 
                               Sphere.cc
                               StdStreamHandlers.cc 
                               UtilString.cc )

add_library( OpenMVG_gui_utils STATIC ${OPENMVG_GUI_UTILS_HEADERS} ${OPENMVG_GUI_UTILS_SOURCES} )
target_include_directories( OpenMVG_gui_utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ )
target_link_libraries( OpenMVG_gui_utils Qt5::Widgets )

set_target_properties( OpenMVG_gui_utils PROPERTIES CXX_STANDARD 11 )
set_property(TARGET OpenMVG_gui_utils PROPERTY FOLDER OpenMVG/software/OpenMVG_gui)
