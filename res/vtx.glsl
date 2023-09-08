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
	vec3 col = vec3(1,1,1);
	gl_PointSize = 10;
	// copied from the infinite shader
	float normalX = (normal.x * 2.001956939697265625) + (-1.001956939697265625);
	float normalY = (normal.y * 2.001956939697265625) + (-1.001956939697265625);
	float normalZ = (normal.z * 2.001956939697265625) + (-1.001956939697265625);
	float normalXClamped = isnan(-1.0) ? normalX : (isnan(normalX) ? (-1.0) : max(normalX, -1.0));
	float normalYClamped = isnan(-1.0) ? normalY : (isnan(normalY) ? (-1.0) : max(normalY, -1.0));
	float normalZClamped = isnan(-1.0) ? normalZ : (isnan(normalZ) ? (-1.0) : max(normalZ, -1.0));
	
	float normalW = (normal.w * 3.0) + (-2.0);
	float normalWClamped = isnan(-1.0) ? normalW : (isnan(normalW) ? (-1.0) : max(normalW, -1.0));
	float someNormalClampedThingy = float(int(uint(normalWClamped > 0.0) - uint(normalWClamped < 0.0)));
	//norm = vec3(inverse(transpose(mat3(TransformMat))) * normalize(normal.xyz));
	norm = vec3(normalXClamped,normalYClamped,normalZClamped);
	
	unicolor.r = col.r * someNormalClampedThingy;
	unicolor.g = col.g * (-someNormalClampedThingy);
	unicolor.b = col.b;
	
	fragPos = vec3( TransformMat * vec4(pos,1.0));
	viewFragPos = vec3(worldViewMat * TransformMat * vec4(pos,1.0));
}

