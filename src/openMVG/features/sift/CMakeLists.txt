
UNIT_TEST(openMVG sift "openMVG_image;openMVG_features;openMVG_system")

if( OpenMVG_BUILD_GPU_TOOLS )
  add_subdirectory( gpu )
endif()