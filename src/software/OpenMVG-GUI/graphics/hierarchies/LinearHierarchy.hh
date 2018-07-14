// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_HIERARCHIES_LINEAR_HIERARCHY_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_HIERARCHIES_LINEAR_HIERARCHY_HH_

#include "SceneHierarchy.hh"

#include <vector>

namespace openMVG_gui
{

class SceneManager;

/**
 * @brief class holding a linear hierarchy (ie: a soup of object without structure)
 */
class LinearHierarchy : public SceneHierarchy
{
public:
  ~LinearHierarchy();

  /**
   * @brief add an object to the hierarchy
   */
  void addObject( std::shared_ptr<RenderableObject> obj ) override;

  /**
   * @brief remove an object from the hierarchy
   * @note if object does not exists in the hierarchy, do nothing
   */
  void removeObject( std::shared_ptr<RenderableObject> obj ) override;

  /**
   * @brief remove all objects that are point clouds
   */
  void removePointClouds() override;

  /**
   * @brief Clear the scene
   */
  void clear( void ) override;

  /**
   * @brief Build internal structure
   */
  void prepare( void ) override;

  /**
   * @brief Render current scene
   * @param scn Scene
   * @param ratio Aspect ratio
   */
  void render( std::shared_ptr<SceneManager> scn, const double w, const double h ) override;

  /**
   * @brief Perform intersection with the scene
   * @param ray The ray used as intersection primitive
   * @retval nullptr if no intersection
   * @retval the nearest object with the ray origin if an intersection exists
   */
  Intersection intersect( const Ray &ray ) const override;

  /**
   * @brief destroy all openGL data (if any present)
   */
  void destroyGLData( void ) override;

private:
  std::vector<std::shared_ptr<RenderableObject>> m_objects;
};

} // namespace openMVG_gui

#endif