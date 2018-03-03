// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2017 Romuald PERROT.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "LinearHierarchy.hh"

#include "SceneManager.hh"
#include "ShaderProgram.hh"

#include "objects/PointCloud.hh"

namespace openMVG_gui
{

LinearHierarchy::~LinearHierarchy()
{
  destroyGLData() ;
  m_objects.clear() ;
}


/**
* @brief add an object to the hierarchy
*/
void LinearHierarchy::addObject( std::shared_ptr<RenderableObject> obj )
{
  m_objects.emplace_back( obj ) ;
}

/**
* @brief remove an object from the hierarchy
* @note if object does not exists in the hierarchy, do nothing
*/
void LinearHierarchy::removeObject( std::shared_ptr<RenderableObject> obj )
{
  auto it = m_objects.begin() ;
  while( it != m_objects.end() )
  {
    if( it->get() == obj.get() )
    {
      *it = nullptr ;
      it = m_objects.erase( it ) ;
    }
    else
    {
      ++it ;
    }
  }
}

/**
 * @brief remove all objects that are point clouds
 */
void LinearHierarchy::removePointClouds()
{
  auto it = m_objects.begin() ;
  while( it != m_objects.end() )
  {
    if( std::dynamic_pointer_cast<PointCloud>( *it ) )
    {
      *it = nullptr ;
      it = m_objects.erase( it ) ;
    }
    else
    {
      ++it ;
    }
  }
}


/**
 * @brief Clear the scene
 */
void LinearHierarchy::clear( void )
{
  m_objects.clear() ;
}

/**
* @brief Build internal structure
*/
void LinearHierarchy::prepare( void )
{
  for( auto & object : m_objects )
  {
    object->prepare() ;
  }
}

/**
* @brief Render current scene
* @param mgr Scene manager (containing camera and lights)
*/
void LinearHierarchy::render( std::shared_ptr<SceneManager> scn , const double w , const double h )
{
  if( ! scn )
  {
    return ;
  }


  // TODO : get resolution for rendering
  std::shared_ptr<Camera> cam = scn->camera() ;
  if( ! cam )
  {
    return ;
  }
  const openMVG::Mat4 camViewMat = cam->viewMatrix() ;
  const openMVG::Mat4 camProjMat = cam->projMatrix( w , h ) ;

  // TODO : compute MVP matrix here to avoid computation in shader

  for( const auto & object : m_objects )
  {
    if( object->isVisible() )
    {
      std::shared_ptr<ShaderProgram> pgm = object->shader() ;
      if( ! pgm )
      {
        continue ;
      }
      pgm->enable() ;

      const openMVG::Mat4 modelMat = object->modelMat() ;

      // 1 - pass uniforms to the shader
      if( pgm->hasUniform( "uProjMat" ) )
      {
        pgm->setUniform( "uProjMat" , camProjMat ) ;
      }
      if( pgm->hasUniform( "uViewMat" ) )
      {
        pgm->setUniform( "uViewMat" , camViewMat ) ;
      }
      if( pgm->hasUniform( "uModelMat" ) )
      {
        pgm->setUniform( "uModelMat" , modelMat ) ;
      }
      if( pgm->hasUniform( "uUseUniformColor" ) )
      {
        pgm->setUniform( "uUseUniformColor" , ( int ) 0 ) ;
        pgm->setUniform( "uColor" , openMVG::Vec3( 1.0 , 1.0 , 1.0 ) ) ;
      }

      // 2 - render this object itself
      {
        object->draw() ;
      }
      pgm->disable() ;
    }
  }
}

/**
* @brief destroy all openGL data (if any present)
*/
void LinearHierarchy::destroyGLData( void )
{
  /*
  for( auto & object : m_objects )
  {
    object->destroyGLData() ;
  }
  */
}

} // openMVG_gui