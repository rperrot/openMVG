
UNIT_TEST(openMVG spectral_sift "openMVG_image;openMVG_features;openMVG_system")

add_executable(openMVG_main_exportSpectralBasis main_exportSpectralBasis.cpp)
target_link_libraries(openMVG_main_exportSpectralBasis
  PRIVATE
    openMVG_features
    openMVG_image
    openMVG_system
    ${OPENMVG_LIBRARY_DEPENDENCIES}
)
set_property(TARGET openMVG_main_exportSpectralBasis PROPERTY FOLDER OpenMVG/features)
install(TARGETS openMVG_main_exportSpectralBasis DESTINATION ${CMAKE_INSTALL_BINDIR})
