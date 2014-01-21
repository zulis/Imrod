#version 330

uniform vec3 CameraPosition;
uniform mat4x4 ViewProjectionMatrix;

in vec3 vert_Position;

out vec3 TexCoord;

void main()
{
    TexCoord = vec3(vert_Position.x, vert_Position.yz);
    gl_Position = ViewProjectionMatrix * vec4(vert_Position + CameraPosition, 1.0);
}
