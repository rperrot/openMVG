set( OPENMVG_IMAGE_GPU_HDRS image_gpu_arithmetics.hpp 
                            image_gpu_convolution.hpp
                            image_gpu_filtering.hpp
                            image_gpu_interface.hpp )
set( OPENMVG_IMAGE_GPU_SRCS image_gpu_arithmetics.cpp
                            image_gpu_convolution.cpp 
                            image_gpu_filtering.cpp
                            image_gpu_interface.cpp )

#file(GLOB_RECURSE REMOVEFILESUNITTEST *_test.cpp)
#list(REMOVE_ITEM OPENMVG_IMAGE_GPU_SRCS ${REMOVEFILESUNITTEST})

add_library( openMVG_image_gpu ${OPENMVG_IMAGE_GPU_SRCS} ${OPENMVG_IMAGE_GPU_HDRS} )

set_target_properties(openMVG_image_gpu PROPERTIES SOVERSION ${OPENMVG_VERSION_MAJOR} VERSION "${OPENMVG_VERSION_MAJOR}.${OPENMVG_VERSION_MINOR}")
install(TARGETS openMVG_image_gpu DESTINATION lib EXPORT openMVG-targets)

add_dependencies( openMVG_image_gpu openMVG_system_gpu )

target_link_libraries( openMVG_image_gpu
  openMVG_image
  openMVG_numeric 
  openMVG_system_gpu )

UNIT_TEST(openMVG image_gpu_interface "openMVG_image_gpu;openMVG_system_gpu")

add_subdirectory( kernels ) 