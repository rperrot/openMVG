set( OPENMVG_SIFT_GPU_SRCS hierarchical_gaussian_scale_space_gpu.cpp )
set( OPENMVG_SIFT_GPU_HDRS hierarchical_gaussian_scale_space_gpu.hpp )

add_library( openMVG_sift_gpu ${OPENMVG_SIFT_GPU_SRCS} ${OPENMVG_SIFT_GPU_HDRS} )

set_target_properties(openMVG_sift_gpu PROPERTIES SOVERSION ${OPENMVG_VERSION_MAJOR} VERSION "${OPENMVG_VERSION_MAJOR}.${OPENMVG_VERSION_MINOR}")
install(TARGETS openMVG_sift_gpu DESTINATION lib EXPORT openMVG-targets)

add_dependencies( openMVG_sift_gpu openMVG_image_gpu openMVG_system_gpu )

target_link_libraries( openMVG_sift_gpu 
  openMVG_image_gpu
  openMVG_image
  openMVG_numeric 
  openMVG_system_gpu )


  UNIT_TEST(openMVG hierarchical_gaussian_scale_space_gpu "openMVG_image;openMVG_features;openMVG_system;openMVG_image_gpu;openMVG_system_gpu;openMVG_sift_gpu")
