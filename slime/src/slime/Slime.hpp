#pragma once

#include <dubu_opengl_app/dubu_opengl_app.hpp>
#include <glm/glm.hpp>

#include "slime/ShaderProgram.hpp"

struct Agent {
	glm::vec2 position;
	float     angle;
	float     _padding;
};

struct Parameters {
	float     movementSpeed  = 10.0f;
	float     decaySpeed     = 0.1f;
	float     rotationAngle  = glm::radians(45.0f);
	float     sensorAngle    = glm::radians(22.5f);
	float     sensorDistance = 9.0f;
	int       sensorSize     = 3;
	glm::vec2 _padding;
};

class Slime : public dubu::opengl_app::AppBase {
public:
	Slime();
	virtual ~Slime() {}

protected:
	virtual void Init() override;

	virtual void Update() override;

private:
	void DrawDockSpace();

	void CheckErrors(std::optional<std::string> err);

	std::unique_ptr<ShaderProgram> mClearProgram;
	std::unique_ptr<ShaderProgram> mTrailMapProgram;
	std::unique_ptr<ShaderProgram> mDecayProgram;
	GLuint                         mTrailMap;
	GLuint                         mAgentBuffer;
	GLuint                         mParameterBuffer;

	float      mSimulationSpeed = 1.0f;
	Parameters mParameters      = {};
};