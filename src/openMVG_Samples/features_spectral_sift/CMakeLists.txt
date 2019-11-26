add_executable( openMVG_sample_features_spectral_sift_gen_eigenimages gen_eigenimages.cpp )
target_link_libraries( openMVG_sample_features_spectral_sift_gen_eigenimages 
                       openMVG_numeric 
                       ${STLPLUS_LIBRARY} )

set_property( TARGET openMVG_sample_features_spectral_sift_gen_eigenimages PROPERTY FOLDER OpenMVG/Samples )
