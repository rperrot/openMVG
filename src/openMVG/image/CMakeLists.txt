
file(
  GLOB
  image_files_header
  *.hpp
)
file(
  GLOB
  image_files_cpp
  *.cpp
)
file(GLOB REMOVEFILESUNITTEST *_test.cpp)

#Remove the future main files
list(REMOVE_ITEM image_files_cpp ${REMOVEFILESUNITTEST})

file(GLOB_RECURSE REMOVEFILESGPUSRCS *_gpu_*.cpp )
list(REMOVE_ITEM image_files_cpp ${REMOVEFILESGPUSRCS} )

file(GLOB_RECURSE REMOVEFILESGPUHDRS *_gpu_*.hpp )
list(REMOVE_ITEM image_files_header ${REMOVEFILESGPUHDRS} )


add_library(openMVG_image ${image_files_header} ${image_files_cpp})
target_link_libraries(
  openMVG_image
    openMVG_numeric
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${TIFF_LIBRARIES})
target_include_directories(
  openMVG_image
    PRIVATE
      ${JPEG_INCLUDE_DIR}
      ${PNG_INCLUDE_DIRS}
      ${TIFF_INCLUDE_DIR})
set_target_properties(openMVG_image PROPERTIES SOVERSION ${OPENMVG_VERSION_MAJOR} VERSION "${OPENMVG_VERSION_MAJOR}.${OPENMVG_VERSION_MINOR}")
install(TARGETS openMVG_image DESTINATION lib EXPORT openMVG-targets)

UNIT_TEST(openMVG image "openMVG_image")
UNIT_TEST(openMVG image_drawing "openMVG_image")
UNIT_TEST(openMVG image_integral "openMVG_image")
UNIT_TEST(openMVG image_io "openMVG_image")
UNIT_TEST(openMVG image_filtering "openMVG_image")
UNIT_TEST(openMVG image_resampling "openMVG_image")

if( OpenMVG_BUILD_GPU_TOOLS )
  add_subdirectory( gpu )
endif()