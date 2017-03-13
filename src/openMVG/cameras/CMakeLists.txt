set( CAMERA_SRCS Camera_Intrinsics.cpp Camera_IO.cpp Camera_Pinhole_Brown.cpp Camera_Pinhole_Fisheye.cpp Camera_Pinhole_Radial.cpp Camera_Pinhole.cpp Cameras_Common_command_line_helper.cpp PinholeCamera.cpp )
set( CAMERA_HDRS Camera_Intrinsics.hpp Camera_IO.hpp Camera_Pinhole_Brown.hpp Camera_Pinhole_Fisheye.hpp Camera_Pinhole_Radial.hpp Camera_Pinhole.hpp Cameras_Common_command_line_helper.hpp PinholeCamera.hpp )


set_source_files_properties(${CAMERA_SRCS} PROPERTIES LANGUAGE CXX)
add_library(openMVG_camera ${CAMERA_SRCS} ${CAMERA_HDRS})

set_target_properties(openMVG_camera PROPERTIES SOVERSION ${OPENMVG_VERSION_MAJOR} VERSION "${OPENMVG_VERSION_MAJOR}.${OPENMVG_VERSION_MINOR}")
install(TARGETS openMVG_camera DESTINATION lib EXPORT openMVG-targets)
set_property(TARGET openMVG_camera PROPERTY FOLDER OpenMVG/OpenMVG)


UNIT_TEST(openMVG Camera_IO "openMVG_multiview;stlplus;openMVG_camera")

UNIT_TEST(openMVG Camera_Pinhole "openMVG_multiview;openMVG_camera")

UNIT_TEST(openMVG Camera_Pinhole_Radial "openMVG_multiview;openMVG_camera")

UNIT_TEST(openMVG Camera_Pinhole_Brown "openMVG_multiview;openMVG_camera")

UNIT_TEST(openMVG Camera_Pinhole_Fisheye "openMVG_multiview;openMVG_camera")

UNIT_TEST(openMVG Camera_Subset_Parametrization "openMVG_multiview;openMVG_camera")


