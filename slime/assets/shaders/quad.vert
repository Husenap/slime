#version 460

in vec2 vPos;

out vec2 aPos;

void main() {
	gl_Position = vec4(vPos, 1.0, 1.0);
	aPos        = vPos;
}