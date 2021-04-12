#include "Slime.hpp"

#include <dubu_pack/dubu_pack.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

#include "slime/Shader.hpp"

constexpr glm::ivec2 LATTICE_SIZE{1024, 1024};
constexpr int        AGENT_COUNT = 1 << 20;

Slime::Slime()
    : dubu::opengl_app::AppBase(
          {.appName = "Slime Simulation", .swapInterval = 0}) {}

void Slime::Init() {
	dubu::pack::Package assetsPackage("assets");

	{  // Compile Lattice Shader
		ComputeShader computeShader(*assetsPackage.GetFileLocator()->ReadFile(
		    "shaders/clear_trail_map.comp"));
		mClearProgram = std::make_unique<ShaderProgram>(computeShader);
		CheckErrors(mClearProgram->GetError());
	}

	{  // Compile TrailMap Shader
		ComputeShader computeShader(*assetsPackage.GetFileLocator()->ReadFile(
		    "shaders/trail_map.comp"));
		mTrailMapProgram = std::make_unique<ShaderProgram>(computeShader);
		CheckErrors(mTrailMapProgram->GetError());
	}

	{  // Compile Decay Shader
		ComputeShader computeShader(
		    *assetsPackage.GetFileLocator()->ReadFile("shaders/decay.comp"));
		mDecayProgram = std::make_unique<ShaderProgram>(computeShader);
		CheckErrors(mDecayProgram->GetError());
	}

	mTrailMap1        = CreateTrailMap();
	mTrailMap2        = CreateTrailMap();
	mTrailMap         = &mTrailMap1;
	mDiffusedTrailMap = &mTrailMap2;

	std::vector<Agent> agents(AGENT_COUNT);
	for (auto& agent : agents) {
		agent.position = glm::diskRand(100.f) + glm::vec2(LATTICE_SIZE) * 0.5f;
		agent.angle    = glm::linearRand(0.f, glm::two_pi<float>());
	}

	glGenBuffers(1, &mAgentBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mAgentBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
	             sizeof(Agent) * AGENT_COUNT,
	             agents.data(),
	             GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mAgentBuffer);

	glGenBuffers(1, &mParameterBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, mParameterBuffer);
	glBufferData(
	    GL_UNIFORM_BUFFER, sizeof(Parameters), &mParameters, GL_STATIC_DRAW);
}

void Slime::Update() {
	static float previousTime = static_cast<float>(glfwGetTime());
	float        currentTime  = static_cast<float>(glfwGetTime());
	float        deltaTime =
	    glm::min(currentTime - previousTime, 1.f / 60.f) * mSimulationSpeed;
	previousTime = currentTime;

	SwapTrailMaps();

	{  // Update Parameters Buffer
		glBindBuffer(GL_UNIFORM_BUFFER, mParameterBuffer);
		glBufferData(GL_UNIFORM_BUFFER,
		             sizeof(Parameters),
		             &mParameters,
		             GL_STATIC_DRAW);
	}

	{  // Evaluate TrailMap
		glBindImageTexture(
		    0, *mTrailMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glUseProgram(mTrailMapProgram->mProgram);
		glUniform1f(0, deltaTime);
		glUniform2iv(1, 1, glm::value_ptr(LATTICE_SIZE));
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, mParameterBuffer);
		glDispatchCompute(AGENT_COUNT, 1, 1);
	}

	{  // Evaluate Decay
		glBindImageTexture(
		    0, *mTrailMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(
		    1, *mDiffusedTrailMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glUseProgram(mDecayProgram->mProgram);
		glUniform1f(0, deltaTime);
		glUniform2iv(1, 1, glm::value_ptr(LATTICE_SIZE));
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, mParameterBuffer);
		glDispatchCompute(LATTICE_SIZE.x, LATTICE_SIZE.y, 1);
	}

	DrawDockSpace();

	if (ImGui::Begin("Viewport")) {
		ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
		ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
		ImVec2 offset    = regionMin;
		ImVec2 regionSize =
		    ImVec2(regionMax.x - regionMin.x, regionMax.y - regionMin.y);
		ImVec2 imageSize = {static_cast<float>(LATTICE_SIZE.x),
		                    static_cast<float>(LATTICE_SIZE.y)};

		float regionRatio = regionSize.x / regionSize.y;
		float imageRatio  = static_cast<float>(LATTICE_SIZE.x) /
		                   static_cast<float>(LATTICE_SIZE.y);

		if (regionRatio > imageRatio) {
			imageSize.x *= regionSize.y / imageSize.y;
			imageSize.y = regionSize.y;
		} else {
			imageSize.y *= regionSize.x / imageSize.x;
			imageSize.x = regionSize.x;
		}

		ImGui::SetCursorPosX((regionSize.x - imageSize.x) * 0.5f + offset.x);
		ImGui::SetCursorPosY((regionSize.y - imageSize.y) * 0.5f + offset.y);

		ImGui::Image(
		    reinterpret_cast<void*>(static_cast<intptr_t>(*mDiffusedTrailMap)),
		    imageSize,
		    {0, 1},
		    {1, 0});
	}
	ImGui::End();

	if (ImGui::Begin("Parameters")) {
		ImGui::DragFloat(
		    "Simulation Speed", &mSimulationSpeed, 0.1f, 0.f, 100.f);

		ImGui::DragFloat(
		    "Movement Speed", &mParameters.movementSpeed, 0.1f, 0.f, 100.f);
		ImGui::DragFloat("Decay Rate", &mParameters.decayRate, 0.1f, 0.f, 50.f);
		ImGui::DragFloat(
		    "Diffuse Rate", &mParameters.diffuseRate, 0.1f, 0.f, 50.f);
		ImGui::SliderAngle(
		    "Rotation Angle", &mParameters.rotationAngle, 0.f, 180.f);
		ImGui::SliderAngle(
		    "Sensor Angle", &mParameters.sensorAngle, 0.f, 180.f);
		ImGui::DragFloat("Sensor Distance",
		                 &mParameters.sensorDistance,
		                 0.1f,
		                 static_cast<float>(mParameters.sensorSize),
		                 100.f);
		ImGui::DragInt("Sensor Size", &mParameters.sensorSize, 0.1f, 1, 2);
	}
	ImGui::End();

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Slime::SwapTrailMaps() {
	auto temp         = mTrailMap;
	mTrailMap         = mDiffusedTrailMap;
	mDiffusedTrailMap = temp;
}

GLuint Slime::CreateTrailMap() {
	GLuint trailMap;

	glGenTextures(1, &trailMap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, trailMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             GL_RGBA32F,
	             LATTICE_SIZE.x,
	             LATTICE_SIZE.y,
	             0,
	             GL_RGBA,
	             GL_FLOAT,
	             NULL);
	glBindImageTexture(0, trailMap, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	{  // Clear TrailMap with black
		glUseProgram(mClearProgram->mProgram);
		glDispatchCompute(LATTICE_SIZE.x, LATTICE_SIZE.y, 1);
	}

	return trailMap;
}

void Slime::DrawDockSpace() {
	ImGuiWindowFlags dockSpaceWindowFlags =
	    ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
	    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
	    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
	    ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
	ImGui::Begin("DOCK_SPACE", nullptr, dockSpaceWindowFlags);

	ImGui::DockSpace(ImGui::GetID("DOCK_SPACE_WINDOW"),
	                 {0.f, 0.f},
	                 ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();
	ImGui::PopStyleVar(3);
}

void Slime::CheckErrors(std::optional<std::string> err) {
	if (err) {
		DUBU_LOG_ERROR("{}", *err);
	}
}
