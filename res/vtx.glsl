#version 450 core

layout (location=0) in vec3 pos;
layout (location=5) in vec4 normal;

layout (location=20) uniform mat4 projectionMat;
layout (location=21) uniform mat4 worldViewMat;
layout (location=22) uniform mat4 TransformMat;


uniform vec3 col;
out vec3 unicolor;
out vec2 texcoords;
out vec3 norm;
out vec3 fragPos;
out vec3 viewFragPos;

layout (location=1) in vec2 albcoords;

void main(){
	
	gl_Position = projectionMat * worldViewMat * TransformMat * vec4(pos,1.0);
	unicolor = vec3(0.9,0.9,0.9);
	gl_PointSize = 10;
	//norm = vec3(inverse(transpose(mat3(TransformMat))) * normalize(normal.xyz));
	norm = normalize(normal.xyz);
	fragPos = vec3( TransformMat * vec4(pos,1.0));
	viewFragPos = vec3(worldViewMat * TransformMat * vec4(pos,1.0));
}

