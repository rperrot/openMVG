set( OPENMVG_SYSTEM_GPU_SRCS OpenCLContext.cpp 
                             OpenCLStandardKernels.cpp )
set( OPENMVG_SYSTEM_GPU_HDRS OpenCLContext.hpp 
                             OpenCLDeviceInfos.hpp 
                             OpenCLStandardKernels.hpp )

add_library( openMVG_system_gpu ${OPENMVG_SYSTEM_GPU_SRCS} ${OPENMVG_SYSTEM_GPU_HDRS} )

set_target_properties(openMVG_system_gpu PROPERTIES SOVERSION ${OPENMVG_VERSION_MAJOR} VERSION "${OPENMVG_VERSION_MAJOR}.${OPENMVG_VERSION_MINOR}")
set_property(TARGET openMVG_system_gpu PROPERTY FOLDER OpenMVG/OpenMVG)
install(TARGETS openMVG_system_gpu DESTINATION lib/ EXPORT openMVG-targets)

target_include_directories( openMVG_system_gpu PUBLIC ${OpenCL_INCLUDE_DIR} )
target_link_libraries( openMVG_system_gpu ${OpenCL_LIBRARIES} )
