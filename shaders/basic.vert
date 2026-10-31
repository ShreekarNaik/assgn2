#version 330 core
layout(location=0) in vec4 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec3 vNormal;
layout(location=3) in vec2 vUV;
uniform mat4 uMVP; // world transform, retained name from A1
uniform mat4 vp;
out vec3 position;
out vec3 normal;
out vec4 color;
out vec2 uv;
void main() {
 vec4 world=uMVP*vPosition;
 position=world.xyz;
 normal=transpose(inverse(mat3(uMVP)))*vNormal;
 color=vColor; uv=vUV;
 gl_Position=vp*world;
}
