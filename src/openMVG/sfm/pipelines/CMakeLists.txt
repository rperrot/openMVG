
add_subdirectory(global)
add_subdirectory(sequential)
add_subdirectory(stellar)

UNIT_TEST(openMVG multi_view_lsm_refiner
  "openMVG_sfm;openMVG_image;${STLPLUS_LIBRARY}")
