ADD_DEFINITIONS( -g )

IF(MSVC)
  SET(OPENMVG_SOFTWARE_MVS_BUILD_DIR "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}")
ELSE()
  SET(OPENMVG_SOFTWARE_MVS_BUILD_DIR ${EXECUTABLE_OUTPUT_PATH})
ENDIF()
ADD_DEFINITIONS( -DMVS_BUILD_DIR="${OPENMVG_SOFTWARE_MVS_BUILD_DIR}" )


FIND_PACKAGE( OpenCL )

# DEPTH MAP GENERATION 
SET( OPENMVG_COMPUTE_DEPTH_MAPS_SOURCES Camera.cpp 
                                        DepthMap.cpp 
                                        DepthMapComputationCPU.cpp 
                                        DepthMapComputationOpenCL.cpp
                                        DepthMapComputationParameters.cpp
                                        Image.cpp 
                                        MatchingCost.cpp 
                                        OpenCLUtils.cpp 
                                        OpenCLWrapper.cpp 
                                        Util.cpp 
                                        main_computeDepthMaps.cpp )

SET( OPENMVG_COMPUTE_DEPTH_MAPS_HEADERS Camera.hpp 
                                        DepthMap.hpp 
                                        DepthMapComputationCPU.hpp 
                                        DepthMapComputationOpenCL.hpp
                                        DepthMapComputationParameters.hpp
                                        Image.hpp 
                                        MatchingCost.hpp
                                        OpenCLUtils.hpp 
                                        OpenCLWrapper.hpp 
                                        Util.hpp )

ADD_EXECUTABLE( openMVG_main_computeDepthMaps ${OPENMVG_COMPUTE_DEPTH_MAPS_HEADERS} ${OPENMVG_COMPUTE_DEPTH_MAPS_SOURCES} ) 
target_include_directories( openMVG_main_computeDepthMaps PRIVATE ${OpenCL_INCLUDE_DIRS} )
target_link_libraries( openMVG_main_computeDepthMaps openMVG_image openMVG_multiview openMVG_sfm openMVG_features ${OpenCL_LIBRARIES} )


# FUSION
SET( OPENMVG_FUSION_DEPTH_MAPS_SOURCES Camera.cpp  
                                       DepthMap.cpp 
                                       DepthMapComputationParameters.cpp
                                       DepthMapFusionParameters.cpp 
                                       Image.cpp 
                                       PointCloud.cpp 
                                       PutativePoint.cpp 
                                       Util.cpp 
                                       main_fusionDepthMaps.cpp )

SET( OPENMVS_FUSION_DEPTH_MAPS_HEADERS Camera.hpp 
                                       DepthMap.hpp 
                                       DepthMapComputationParameters.hpp 
                                       DepthMapFusionParameters.hpp 
                                       Image.hpp
                                       PointCloud.hpp 
                                       PutativePoint.hpp 
                                       Util.hpp ) 


ADD_EXECUTABLE( openMVG_main_fusionDepthMaps ${OPENMVS_FUSION_DEPTH_MAPS_HEADERS} ${OPENMVG_FUSION_DEPTH_MAPS_SOURCES} ) 
target_include_directories( openMVG_main_fusionDepthMaps PRIVATE ${OpenCL_INCLUDE_DIRS} )
target_link_libraries( openMVG_main_fusionDepthMaps openMVG_image openMVG_multiview openMVG_sfm ${OpenCL_LIBRARIES} )




# Copy kernel to destination folder 
add_custom_command( TARGET openMVG_main_computeDepthMaps PRE_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/opencl_kernels.cl ${OPENMVG_SOFTWARE_MVS_BUILD_DIR} )