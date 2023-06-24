#version 450 core

out vec4 color;
in vec3 unicolor;
in vec2 texcoords;
in vec3 norm;
in vec3 fragPos;
in vec3 viewFragPos;

void main(){
	vec3 lightpos = vec3(20.0,50.0,50.0);
	//vec3 light_dir = normalize(lightpos - fragPos);
	vec3 light_dir = normalize(vec3(1,1,1));
	color = vec4(1.0,1.0,1.0,1.0);
	color.rgb *= max(dot(light_dir, norm),0.15);
	//color.rgb *= max(abs(dot(normalize(viewFragPos), norm)),0.15);
}

