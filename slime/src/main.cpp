#include <dubu_pack/dubu_pack.h>
#include <dubu_window/dubu_window.h>

#include "slime/Shader.h"
#include "slime/ShaderProgram.h"

float vertices[6] = {
    -1.0f,
    3.0f,
    3.0f,
    -1.0f,
    -1.0f,
    -1.0f,
};

void CheckErrors(std::optional<std::string> err) {
	if (err) {
		std::cerr << *err << std::endl;
	}
}

int main() {
	auto window = std::make_unique<dubu::window::GLFWWindow>(
	    dubu::window::GLFWWindow::CreateInfo{
	        .width  = 512,
	        .height = 512,
	        .title  = "Slime",
	        .api    = dubu::window::GLFWWindow::Api::OpenGL,
	    });

	glfwMakeContextCurrent(window->GetGLFWHandle());
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		std::cerr << "Failed to init glad" << std::endl;
		return 1;
	}

	auto resizeToken = window->RegisterListener<dubu::window::EventResize>(
	    [](const auto& e) { glViewport(0, 0, e.width, e.height); });
	auto keyPressToken = window->RegisterListener<dubu::window::EventKeyPress>(
	    [&](const auto& e) {
		    if (e.key == GLFW_KEY_ESCAPE) {
			    glfwSetWindowShouldClose(window->GetGLFWHandle(), GLFW_TRUE);
		    }
	    });

	glViewport(0, 0, 512, 512);
	glfwSwapInterval(0);

	GLuint vertexArray;
	glGenBuffers(1, &vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexArray);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vertexArray);

	int    texWidth  = 512;
	int    texHeight = 512;
	GLuint texOutput;
	glGenTextures(1, &texOutput);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texOutput);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             GL_RGBA32F,
	             texWidth,
	             texHeight,
	             0,
	             GL_RGBA,
	             GL_FLOAT,
	             NULL);
	glBindImageTexture(0, texOutput, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	dubu::pack::Package assetsPackage("assets");

	VertexShader copyVertexShader(
	    *assetsPackage.GetFileLocator()->ReadFile("shaders/quad.vert"));
	CheckErrors(copyVertexShader.GetError());

	FragmentShader copyFragmentShader(
	    *assetsPackage.GetFileLocator()->ReadFile("shaders/quad.frag"));
	CheckErrors(copyFragmentShader.GetError());

	ShaderProgram copyProgram(copyVertexShader, copyFragmentShader);
	CheckErrors(copyProgram.GetError());

	ComputeShader computeShader(
	    *assetsPackage.GetFileLocator()->ReadFile("shaders/slime.comp"));
	CheckErrors(computeShader.GetError());

	ShaderProgram computeProgram(computeShader);
	CheckErrors(computeProgram.GetError());

	GLint vPosLoc = copyProgram.GetAttributeLocation("vPos");


	while (!window->ShouldClose()) {
		window->PollEvents();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		{
			glUseProgram(computeProgram.mProgram);
			glDispatchCompute(800, 600, 1);
		}

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		{
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(copyProgram.mProgram);
			glBindVertexArray(vertexArray);
			glVertexAttribPointer(vPosLoc,
			                      2,
			                      GL_FLOAT,
			                      GL_FALSE,
			                      (GLsizei)sizeof(vertices[0]) * 2,
			                      nullptr);
			glEnableVertexAttribArray(vPosLoc);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texOutput);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		window->SwapBuffers();
	}

	return 0;
}