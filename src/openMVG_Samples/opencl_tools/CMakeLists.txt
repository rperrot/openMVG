add_executable( openMVG_sample_main_OpenCL_infos main.cpp )

target_link_libraries( openMVG_sample_main_OpenCL_infos openMVG_system_gpu )

set_property( TARGET openMVG_sample_main_OpenCL_infos PROPERTY FOLDER OpenMVG/Samples )
