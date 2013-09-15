//GLSL

varying vec2 texcoord0; 

void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  texcoord0 = vec2(gl_MultiTexCoord0[0], gl_MultiTexCoord0[1]);
}