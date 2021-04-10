#include <dubu_opengl_app/dubu_opengl_app.hpp>
#include <dubu_pack/dubu_pack.h>
#include <imgui/imgui.h>

#include "slime/Shader.h"
#include "slime/ShaderProgram.h"

constexpr int WIDTH  = 1920;
constexpr int HEIGHT = 1080;

class SlimeApp : public dubu::opengl_app::AppBase {
public:
	SlimeApp()
	    : dubu::opengl_app::AppBase({.appName = "Slime Simulation"}) {}
	virtual ~SlimeApp() {}

protected:
	virtual void Init() override {
		dubu::pack::Package assetsPackage("assets");

		ComputeShader computeShader(
		    *assetsPackage.GetFileLocator()->ReadFile("shaders/slime.comp"));
		CheckErrors(computeShader.GetError());

		mSlimeProgram = std::make_unique<ShaderProgram>(computeShader);
		CheckErrors(mSlimeProgram->GetError());

		glGenTextures(1, &mImage);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mImage);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             GL_RGBA32F,
		             WIDTH,
		             HEIGHT,
		             0,
		             GL_RGBA,
		             GL_FLOAT,
		             NULL);
		glBindImageTexture(
		    0, mImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	}

	virtual void Update() override {
		{
			glUseProgram(mSlimeProgram->mProgram);
			glUniform1f(0, mOffset);
			glUniform2f(1, WIDTH, HEIGHT);
			glUniform1f(2, mFadeSpeed);
			glDispatchCompute(WIDTH, HEIGHT, 1);
		}

		if (ImGui::Begin("Viewport")) {
			ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
			ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
			ImVec2 offset    = regionMin;
			ImVec2 regionSize =
			    ImVec2(regionMax.x - regionMin.x, regionMax.y - regionMin.y);
			ImVec2 imageSize = {WIDTH, HEIGHT};

			float regionRatio = regionSize.x / regionSize.y;
			float imageRatio =
			    static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);

			if (regionRatio > imageRatio) {
				imageSize.x *= regionSize.y / imageSize.y;
				imageSize.y = regionSize.y;
			} else {
				imageSize.y *= regionSize.x / imageSize.x;
				imageSize.x = regionSize.x;
			}

			ImGui::SetCursorPosX((regionSize.x - imageSize.x) * 0.5f +
			                     offset.x);
			ImGui::SetCursorPosY((regionSize.y - imageSize.y) * 0.5f +
			                     offset.y);

			ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(mImage)),
			             imageSize,
			             {0, 1},
			             {1, 0});
		}
		ImGui::End();

		if (ImGui::Begin("Parameters")) {
			ImGui::DragFloat("Offset", &mOffset, 0.01f);
			ImGui::SliderFloat("FadeSpeed", &mFadeSpeed, 0.0001f, 1.0f);
		}
		ImGui::End();

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

private:
	void CheckErrors(std::optional<std::string> err) {
		if (err) {
			std::cerr << *err << std::endl;
		}
	}

	std::unique_ptr<ShaderProgram> mSlimeProgram;
	GLuint                         mImage;
	float                          mOffset = 0.f;
	float                          mFadeSpeed = 0.01f;
};

int main() {
	SlimeApp app;

	app.Run();

	return 0;
}