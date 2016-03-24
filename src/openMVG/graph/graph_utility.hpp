// Copyright (c) 2016 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_GRAPH_GRAPH_UTILITY_HPP_
#define _OPENMVG_GRAPH_GRAPH_UTILITY_HPP_

namespace openMVG
{
namespace graph
{

/**
 * @brief Class containing no data (for minimum storage)
 */
struct NullData
{
  // Ensure serialization
  template <typename Archive>
  void serialize( Archive & ar )
  {
    // Do nothing
  }

};



}
}

#endif