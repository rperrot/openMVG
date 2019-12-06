add_executable( openMVG_sample_features_spectral_sift_gen_eigenimages gen_eigenimages.cpp )
target_link_libraries( openMVG_sample_features_spectral_sift_gen_eigenimages 
                       openMVG_numeric 
                       ${STLPLUS_LIBRARY} )

add_executable( openMVG_sample_features_spectral_sift spectral_sift_sample.cpp )
target_link_libraries( openMVG_sample_features_spectral_sift 
                       openMVG_numeric 
                       openMVG_image 
                       ${STLPLUS_LIBRARY} )


set_property( TARGET openMVG_sample_features_spectral_sift_gen_eigenimages PROPERTY FOLDER OpenMVG/Samples )
set_property( TARGET openMVG_sample_features_spectral_sift                 PROPERTY FOLDER OpenMVG/Samples )
