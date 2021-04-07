#version 460

in vec2 aPos;

out vec4 frag_color;

layout(binding = 0) uniform sampler2D img_output;

void main() {
	vec2 uv    = aPos.xy * 0.5 + 0.5;
	frag_color = texture(img_output, uv);
}
