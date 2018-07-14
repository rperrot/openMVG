// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SCENE_MANAGER_HH_
#define _OPENMVG_SOFTWARE_OPENMVG_GUI_GRAPHICS_SCENE_MANAGER_HH_

#include "Camera.hh"
#include "SceneHierarchy.hh"

#include <memory>

namespace openMVG_gui
{

/**
 * @brief class used to manage a render scene
 * contains objects
 */
class SceneManager : public std::enable_shared_from_this<SceneManager>
{
public:
  /**
   * @brief Ctr
   * @param cam Camera
   * @param s_hier Scene hierarchy manager
   */
  SceneManager( std::shared_ptr<Camera> cam, std::shared_ptr<SceneHierarchy> s_hier );

  ~SceneManager( void );

  /**
   * @brief add object to scene
   */
  void addObject( std::shared_ptr<RenderableObject> obj );

  /**
   * @brief remove object from scene
   */
  void removeObject( std::shared_ptr<RenderableObject> obj );

  /**
   * @brief Remove point clouds
   */
  void removePointClouds( void );

  /**
   * @brief Clear all scene
   */
  void clear( void );

  /**
   * @brief Get camera
   * @return current camera
   */
  std::shared_ptr<Camera> camera( void ) const;

  /**
   * @brief Set camera
   * @param cam New camera
   */
  void setCamera( std::shared_ptr<Camera> cam );

  /**
   * @brief prepare scene before rendering
   */
  void prepare( void );

  /**
   * @brief render current scene
   */
  void render( const double w, const double h );

  /**
   * @brief Perform intersection with the scene from all the objects inside
   * @param ray The ray used as support of the intersection
   * @retval nullptr if no object is bellow the ray
   * @retval the first object under the click
   */
  Intersection intersect( const Ray &ray ) const;

  // Set cameras gizmos
  void setCameraGizmos( const std::map<int, std::shared_ptr<RenderableObject>> &objs );

  // Remove all camera gizmos
  void removeCameraGizmos( void );

  // Get a gizmo
  std::shared_ptr<RenderableObject> cameraGizmo( const int id ) const;

  // Get id of a given camera gizmo
  int cameraGizmoId( std::shared_ptr<RenderableObject> gizmo ) const;

  // Get list of camera gizmos
  std::vector<std::shared_ptr<RenderableObject>> cameraGizmos( void ) const;

  /**
   * @brief destroy all openGL data (if any present)
   */
  void destroyGLData( void );

private:
  std::map<int, std::shared_ptr<RenderableObject>> m_camera_gizmos;
  std::shared_ptr<SceneHierarchy>                  m_hierarchy;
  std::shared_ptr<Camera>                          m_camera;
};

} // namespace openMVG_gui

#endif