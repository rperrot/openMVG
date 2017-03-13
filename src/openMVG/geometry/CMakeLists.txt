set( GEOMETRY_SRCS box.cpp frustum.cpp half_space_intersection.cpp pose3.cpp rigid_transformation3D_srt.cpp Similarity3.cpp Similarity3_Kernel.cpp )
set( GEOMETRY_HDRS box.hpp frustum.hpp half_space_intersection.hpp pose3.hpp rigid_transformation3D_srt.hpp Similarity3.hpp Similarity3_Kernel.hpp )


set_source_files_properties(${GEOMETRY_SRCS} PROPERTIES LANGUAGE CXX)
add_library(openMVG_geometry ${GEOMETRY_SRCS} ${GEOMETRY_HDRS})

set_target_properties(openMVG_geometry PROPERTIES SOVERSION ${OPENMVG_VERSION_MAJOR} VERSION "${OPENMVG_VERSION_MAJOR}.${OPENMVG_VERSION_MINOR}")
install(TARGETS openMVG_geometry DESTINATION lib EXPORT openMVG-targets)
set_property(TARGET openMVG_geometry PROPERTY FOLDER OpenMVG/OpenMVG)


UNIT_TEST(openMVG rigid_transformation3D_srt "openMVG_numeric;openMVG_geometry")

UNIT_TEST(openMVG half_space_intersection "openMVG_numeric;openMVG_linearProgramming;openMVG_geometry")

UNIT_TEST(openMVG frustum_intersection "openMVG_numeric;openMVG_multiview_test_data;openMVG_multiview;openMVG_linearProgramming;openMVG_geometry")

UNIT_TEST(openMVG frustum_box_intersection "openMVG_numeric;openMVG_multiview_test_data;openMVG_multiview;openMVG_linearProgramming;openMVG_geometry")


