#version 120
varying vec3 Vertex_LightDir; 
varying vec3 Vertex_EyeVec;
varying vec3 Vertex_Normal;

uniform mat4 gxl3d_ModelViewProjectionMatrix; // Automatically passed by GLSL Hacker
uniform mat4 gxl3d_ModelViewMatrix; // Automatically passed by GLSL Hacker
uniform vec4 light_position;
uniform vec4 uv_tiling;

void main()
{
  gl_Position = gxl3d_ModelViewProjectionMatrix * gl_Vertex;
  gl_TexCoord[0] = gl_MultiTexCoord0 * uv_tiling;
  Vertex_Normal = gl_NormalMatrix * gl_Normal;
  vec4 view_vertex = gxl3d_ModelViewMatrix * gl_Vertex;
  Vertex_LightDir = light_position.xyz - view_vertex.xyz;
  Vertex_EyeVec = -view_vertex.xyz;
}