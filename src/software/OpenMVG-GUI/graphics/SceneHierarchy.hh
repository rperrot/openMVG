// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _SCENE_HIERARCHY_HH_
#define _SCENE_HIERARCHY_HH_

#include "RenderableObject.hh"

namespace openMVG_gui
{

class SceneManager ;

/**
* @brief Class managing a hierarchy
* this contains renderable objects and is responsible for rendering it
* -> Maybe used for culling
*/
class SceneHierarchy
{
  public:

    /**
    * @brief Ctr
    */
    SceneHierarchy( void ) ;

    /**
    * @brief add an object to the hierarchy
    */
    virtual void addObject( std::shared_ptr<RenderableObject> obj ) = 0 ;

    /**
    * @brief remove an object from the hierarchy
    * @note if object does not exists in the hierarchy, do nothing
    */
    virtual void removeObject( std::shared_ptr<RenderableObject> obj ) = 0 ;

    /**
    * @brief Build internal structure
    */
    virtual void prepare( void ) = 0 ;

    /**
    * @brief Render current scene
    * @param mgr Scene manager (containing camera and lights)
    */
    virtual void render( std::shared_ptr<SceneManager> scn , const double w , const double h ) = 0 ;

    /**
    * @brief destroy all openGL data (if any present)
    */
    virtual void destroyGLData( void ) = 0 ;

  private:


} ;

} // namespace openMVG_gui

#endif