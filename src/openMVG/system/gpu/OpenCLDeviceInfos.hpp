// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2018 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_SYSTEM_GPU_OPENCL_DEVICE_INFOS_HPP
#define OPENMVG_SYSTEM_GPU_OPENCL_DEVICE_INFOS_HPP

#ifdef APPLE
  #include <OpenCL/cl.h>
#else
  #include <CL/cl.h>
#endif

#include <string>

namespace openMVG
{
namespace system
{
namespace gpu
{

/**
 * @brief Structure holding basic Device informations
 */
struct OpenCLDeviceInfos
{
  std::string m_name ;
  std::string m_vendor_name ;

  cl_ulong m_max_global_memory_size ;
  cl_uint m_max_clock_frequency ;
  cl_uint m_max_compute_units ;

  bool m_support_image_2D ;
  size_t m_image_2D_max_width ;
  size_t m_image_2D_max_height ;

  bool m_support_image_3D ;
  size_t m_image_3D_max_width ;
  size_t m_image_3D_max_height ;
  size_t m_image_3D_max_depth ;
} ;

} // namespace gpu
} // namespace system
} // namespace openMVG

#endif