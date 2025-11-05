#version 460 
#pragma shader_stage(fragment)

layout(location = 0) in vec2 i_Texcoord;
layout(location = 0) out vec4 o_Color;

layout(set = 0,binding = 0) uniform sampler2D u_Texture;
layout(set = 0,binding = 1) uniform u_Attribute{
	bool isOrthographic;

	float kd;
	float light_w;
	float light_h;
	float light_pos;
	float light_offset_x;
	float light_offset_y;
	float light_power;

	float view_pos;
	
	float panel_pos;

	uint sample_count;
};

const vec3 light_normal = vec3(0.0, 0.0, 1.0);
const vec3 panel_normal = vec3(0.0, 0.0, -1.0);
const float PI = 3.141592653589793;

float random3DTo1D(vec3 seed, float a, vec3 b){
	float random = dot(sin(seed),b);
	return fract(sin(random) * a);
}

vec2 random2D(vec3 seed){
	vec2 random =  vec2 (
		random3DTo1D(seed, 14375.5964, vec3(15.637,76.243,37.168)),
		random3DTo1D(seed, 14684.6034,vec3(45.366, 23.168,65.918))
	);
	return random * 2.0 - 1.0;
}

vec3 samplelight(vec3 seed){
	vec2 random = random2D(seed);
	return vec3(light_w * random.x + light_offset_x, light_h * random.y + light_offset_y, light_pos);
}

float textureIntersect(vec3 inv_wi, vec3 panel_intersect){
	vec3 p0 = vec3(0.0, 0.0, 0.0);
	float t = dot(panel_normal, p0 - panel_intersect)/ dot(panel_normal, inv_wi);
	vec3 sample_pos = panel_intersect + t * inv_wi;
	vec2 uv = sample_pos.xy * 0.5 + 0.5;
	if(uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
		return 1.0;
	else{
		return 1.0 - texture(u_Texture, uv).a;
	}
}

void main(){
	//vec3 tex_ws = vec3(i_Texcoord * 2.0 - 1.0, 0.0);
	vec3 eye_ws = vec3(0.0, 0.0, view_pos);
	vec3 p0 = vec3(0.0, 0.0, panel_pos);
	vec2 texelSize = 1.0 / vec2(textureSize(u_Texture, 0));
	float direct_light = 0.0;


	for(uint i = 0; i < sample_count;i++){
		//扰动世界空间uv坐标位置
		vec3 seed = vec3(float(i),gl_FragCoord.x,gl_FragCoord.y);
		vec2 random = random2D(seed) * texelSize * 0.5;
		vec3 tex_ws = vec3((i_Texcoord + random) * 2.0 - 1.0, 0.0);

		if(isOrthographic)
			eye_ws = vec3(tex_ws.xy,view_pos);
		vec3 viewDir_ws = normalize(tex_ws - eye_ws);
		float t = dot(panel_normal, p0 - eye_ws)/ dot(panel_normal, viewDir_ws);
		vec3 panel_intersect = eye_ws + t * viewDir_ws;

		//光源采样
		seed = vec3(gl_FragCoord.x, sin(float(i)), gl_FragCoord.y);
		vec3 light_sample_pos_ws = samplelight(seed);
		vec3 wi = normalize(panel_intersect - light_sample_pos_ws);
		float weight = textureIntersect(-wi, panel_intersect);

		float pdf_light = 1.0 / (light_w * light_h * 4);
		float diffuse_brdf = kd / PI;

		vec3  d      = panel_intersect - light_sample_pos_ws;
        float dist2  = dot(d, d);

		direct_light += weight * light_power * diffuse_brdf * 
						max(0.0, dot(panel_normal, -wi)) * 
						max(0.0, dot(light_normal, wi))/ 
						dist2 / pdf_light;
	}
	vec4 panel_color = vec4(vec3(direct_light / float(sample_count)), 1.0);
	panel_color = (panel_color*(2.51*panel_color + 0.03))/
					(panel_color*(2.43*panel_color + 0.59) + 0.14);
	panel_color = pow(panel_color,vec4(1.0/2.2));
	vec4 tex_color = texture(u_Texture, i_Texcoord);
	o_Color = tex_color * tex_color.a + panel_color * (1.0 - tex_color.a);
	//o_Color = vec4(random3DTo1D(vec3(float(sample_count),gl_FragCoord.x,gl_FragCoord.y), 14375.5964, vec3(15.637,76.243,37.168)));
}