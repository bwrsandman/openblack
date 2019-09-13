#version 330

uniform sampler2D s_diffuse;

in vec4 v_position;
in vec3 v_normal;
in vec2 v_texcoord0;

out vec4 o_color;

float alphaThreshold = 1.0 / 255.0;
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
vec4 lightPos = vec4(-4000, 1300, -1435, 1.0f);
float ambientStrength = 0.5;

void main()
{
	// ambient
	vec3 ambient = ambientStrength * lightColor;

	// diffuse
	vec3 norm = normalize(v_normal);
	vec4 lightDir = normalize(lightPos - v_position);
	float diff = max(dot(v_normal, lightDir.xyz), 0.0);
	vec3 diffuse = diff * lightColor * 0.5;

	vec4 diffuseTex = texture2D(s_diffuse, v_texcoord0);
	diffuseTex.rgb = diffuseTex.rgb * (ambient + diffuse);
	if(diffuseTex.a <= alphaThreshold) discard;
	o_color = diffuseTex;
}
