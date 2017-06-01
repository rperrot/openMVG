#version 330

layout( location = 0) in vec3 inPos ;
layout( location = 1) in vec3 inCol ; 

uniform mat4 uProjMat ;
uniform mat4 uViewMat ; 
uniform mat4 uModelMat ; 

out vec3 vCol ; 

void main( )
{
  vCol = inCol ;
  gl_Position = uProjMat * uViewMat * uModelMat * vec4( inPos , 1.0 ) ; 
}