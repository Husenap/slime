#pragma once

template <GLenum SHADER_TYPE>
class Shader {
public:
	Shader(const std::vector<char>& shaderCode)
	    : Shader(shaderCode.data(), static_cast<int>(shaderCode.size())) {}
	Shader(const char* shaderCode, int shaderCodeLength = -1) {
		mShader = glCreateShader(SHADER_TYPE);

		glShaderSource(mShader, 1, &shaderCode, &shaderCodeLength);
		glCompileShader(mShader);
	}
	~Shader() { glDeleteShader(mShader); }

	std::optional<std::string> GetError() {
		int success(-1);
		glGetShaderiv(mShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char buffer[4096];
			glGetShaderInfoLog(mShader, 4096, nullptr, buffer);
			return std::string(buffer);
		}
		return std::nullopt;
	}

	GLuint mShader;
};

using VertexShader   = Shader<GL_VERTEX_SHADER>;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;
using ComputeShader  = Shader<GL_COMPUTE_SHADER>;
