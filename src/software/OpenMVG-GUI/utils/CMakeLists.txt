set( OPENMVG_GUI_UTILS_HEADERS BellBall.hh 
                               BoundingSphere.hh 
                               DualQuaternion.hh 
                               Plane.hh 
                               PlyLoader.hh
                               Sphere.hh 
                               UtilString.hh )
set( OPENMVG_GUI_UTILS_SOURCES BellBall.cc 
                               BoundingSphere.cc 
                               DualQuaternion.cc 
                               Plane.cc 
                               PlyLoader.cc 
                               Sphere.cc
                               UtilString.cc )

add_library( OpenMVG_gui_utils STATIC ${OPENMVG_GUI_UTILS_HEADERS} ${OPENMVG_GUI_UTILS_SOURCES} )

set_target_properties( OpenMVG_gui_utils PROPERTIES CXX_STANDARD 11 )
