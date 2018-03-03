#version 330

layout( location = 0) in vec3 inPos ;
layout( location = 1) in vec3 inCol ; 

uniform mat4 uProjMat ;
uniform mat4 uViewMat ; 
uniform mat4 uModelMat ; 

uniform vec3 uColor ; 
uniform bool uUseUniformColor ; 

out vec3 vCol ; 

void main( )
{
  vCol = inCol ; 
  if( uUseUniformColor ) 
  {
    vCol = uColor ; 
  }
  gl_Position = uProjMat * uViewMat * uModelMat * vec4( inPos , 1.0 ) ; 
}