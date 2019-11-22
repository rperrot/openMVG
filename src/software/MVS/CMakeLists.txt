IF(MSVC)
  SET(OPENMVG_SOFTWARE_MVS_BUILD_DIR "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}")
ELSE()
  SET(OPENMVG_SOFTWARE_MVS_BUILD_DIR ${EXECUTABLE_OUTPUT_PATH})
ENDIF()
ADD_DEFINITIONS( -DMVS_BUILD_DIR="${OPENMVG_SOFTWARE_MVS_BUILD_DIR}" )

# ADD_DEFINITIONS( -W -Wall -Wextra )
# ADD_DEFINITIONS( -g )

FIND_PACKAGE( OpenCL )

# DEPTH MAP GENERATION 
SET( OPENMVG_COMPUTE_DEPTH_MAPS_SOURCES Camera.cpp 
                                        CostMetricFunctors.cpp
                                        DepthMap.cpp 
                                        DepthMapComputationCommon.cpp 
                                        DepthMapComputationCPU.cpp 
                                        DepthMapComputationOpenCL.cpp
                                        DepthMapComputationParameters.cpp
                                        Generators.cpp
                                        HypothesisSampler.cpp 
                                        Image.cpp 
                                        MatchingCost.cpp 
                                        OpenCLUtils.cpp 
                                        OpenCLWrapper.cpp 
                                        Util.cpp 
                                        main_computeDepthMaps.cpp )

SET( OPENMVG_COMPUTE_DEPTH_MAPS_HEADERS Camera.hpp
                                        CostMetricFunctors.hpp 
                                        DepthMap.hpp 
                                        DepthMapComputationCommon.hpp 
                                        DepthMapComputationCPU.hpp 
                                        DepthMapComputationOpenCL.hpp
                                        DepthMapComputationParameters.hpp
                                        Generators.hpp
                                        HypothesisSampler.hpp
                                        Image.hpp 
                                        MatchingCost.hpp
                                        OpenCLUtils.hpp 
                                        OpenCLWrapper.hpp 
                                        Util.hpp )

ADD_EXECUTABLE( openMVG_main_computeDepthMaps ${OPENMVG_COMPUTE_DEPTH_MAPS_HEADERS} ${OPENMVG_COMPUTE_DEPTH_MAPS_SOURCES} ) 
target_include_directories( openMVG_main_computeDepthMaps PRIVATE ${OpenCL_INCLUDE_DIRS} )
target_include_directories( openMVG_main_computeDepthMaps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/daisy/include )
target_link_libraries( openMVG_main_computeDepthMaps openMVG_image openMVG_multiview openMVG_sfm openMVG_features ${OpenCL_LIBRARIES} ${STLPLUS_LIBRARY} daisy )

#set_target_properties(openMVG_main_computeDepthMaps PROPERTIES COMPILE_OPTIONS "-Wconversion")

# FUSION
SET( OPENMVG_FUSION_DEPTH_MAPS_SOURCES BoundingBox.cpp 
                                       Camera.cpp  
                                       DepthMap.cpp 
                                       DepthMapComputationParameters.cpp
                                       DepthMapFusionParameters.cpp 
                                       Generators.cpp
                                       Image.cpp 
                                       PointCloud.cpp 
                                       PutativePoint.cpp 
                                       Util.cpp 
                                       main_fusionDepthMaps.cpp 
                                       normal_estimation/HoppeNormalEstimation.cpp )

SET( OPENMVS_FUSION_DEPTH_MAPS_HEADERS BoundingBox.hpp
                                       Camera.hpp 
                                       DepthMap.hpp 
                                       DepthMapComputationParameters.hpp 
                                       DepthMapFusionParameters.hpp 
                                       Generators.hpp
                                       Image.hpp
                                       PointCloud.hpp 
                                       PutativePoint.hpp 
                                       Util.hpp
                                       normal_estimation/AbstractNormalEstimation.hpp 
                                       normal_estimation/HoppeNormalEstimation.hpp ) 

add_subdirectory( daisy )

ADD_EXECUTABLE( openMVG_main_fusionDepthMaps ${OPENMVS_FUSION_DEPTH_MAPS_HEADERS} ${OPENMVG_FUSION_DEPTH_MAPS_SOURCES} ) 
target_include_directories( openMVG_main_fusionDepthMaps PRIVATE ${OpenCL_INCLUDE_DIRS} )
target_include_directories( openMVG_main_fusionDepthMaps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries( openMVG_main_fusionDepthMaps openMVG_image openMVG_multiview openMVG_sfm ${OpenCL_LIBRARIES} ${STLPLUS_LIBRARY} )




# Copy kernels to destination folder 
add_custom_command( TARGET openMVG_main_computeDepthMaps PRE_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/opencl_kernels.cl ${OPENMVG_SOFTWARE_MVS_BUILD_DIR} )
add_custom_command( TARGET openMVG_main_computeDepthMaps PRE_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/opencl_metrics_kernels.cl ${OPENMVG_SOFTWARE_MVS_BUILD_DIR} )
add_custom_command( TARGET openMVG_main_computeDepthMaps PRE_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/opencl_common_kernels.cl ${OPENMVG_SOFTWARE_MVS_BUILD_DIR} )