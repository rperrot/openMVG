add_executable( openMVG_sample_spectral_sift_build_eigenimages build_eigenimages.cpp )
target_link_libraries( openMVG_sample_spectral_sift_build_eigenimages 
                       openMVG_numeric 
                       ${STLPLUS_LIBRARY} )

set_property( TARGET openMVG_sample_features_siftPutative PROPERTY FOLDER OpenMVG/Samples )
