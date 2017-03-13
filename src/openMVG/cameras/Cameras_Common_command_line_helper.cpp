#include "Cameras_Common_command_line_helper.hpp"

#include "openMVG/stl/split.hpp"

#include <iostream>

namespace openMVG
{
namespace cameras
{
// Allow to initialize an object cameras::Intrinsic_Parameter_Type BA from
// a string and delimiters('|')
//


cameras::Intrinsic_Parameter_Type
StringTo_Intrinsic_Parameter_Type
(
  const std::string & rhs
)
{
  // Split the string by the '|' token.
  std::vector<std::string> items;
  stl::split(rhs, '|', items);

  cameras::Intrinsic_Parameter_Type intrinsics_opt =
    static_cast<cameras::Intrinsic_Parameter_Type>(0);

  // Look for the "STRING KEY" parameters and initialize them
  for (const std::string & item : items)
  {
    // cameras::Intrinsic_Parameter_Type
    if (item == "NONE")
    {
      return cameras::Intrinsic_Parameter_Type::NONE;
    }
    else if (item == "ADJUST_FOCAL_LENGTH")
    {
      intrinsics_opt = intrinsics_opt | cameras::Intrinsic_Parameter_Type::ADJUST_FOCAL_LENGTH;
    }
    else if (item == "ADJUST_PRINCIPAL_POINT")
    {
      intrinsics_opt = intrinsics_opt | cameras::Intrinsic_Parameter_Type::ADJUST_PRINCIPAL_POINT;
    }
    else if (item == "ADJUST_DISTORTION")
    {
      intrinsics_opt = intrinsics_opt | cameras::Intrinsic_Parameter_Type::ADJUST_DISTORTION;
    }
    else if (item == "ADJUST_ALL")
    {
      intrinsics_opt = cameras::Intrinsic_Parameter_Type::ADJUST_FOCAL_LENGTH
        | cameras::Intrinsic_Parameter_Type::ADJUST_PRINCIPAL_POINT
        | cameras::Intrinsic_Parameter_Type::ADJUST_DISTORTION;
    }
    else
    {
      std::cerr << "WARNING: Unknow KEY: " << item << std::endl;
      intrinsics_opt = static_cast<cameras::Intrinsic_Parameter_Type>(0);
      break;
    }
  }

  return intrinsics_opt;
}

} // namespace cameras
} // namespace openMVG
