#version 330

out vec4 outCol ; 

in vec3 vCol ; 

void main( void )
{
  outCol = vec4( vCol , 1.0 ) ;
}
