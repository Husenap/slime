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
	float movementSpeed  = 10.0f;
	float decayRate     = 0.1f;
	float diffuseRate   = 1.0f;
	float rotationAngle  = glm::radians(45.0f);
	float sensorAngle    = glm::radians(22.5f);
	float sensorDistance = 9.0f;
	int   sensorSize     = 1;
	float _padding;
};

class Slime : public dubu::opengl_app::AppBase {
public:
	Slime();
	virtual ~Slime() {}

protected:
	virtual void Init() override;

	virtual void Update() override;

private:
	void   SwapTrailMaps();
	GLuint CreateTrailMap();

	void DrawDockSpace();

	void CheckErrors(std::optional<std::string> err);

	std::unique_ptr<ShaderProgram> mClearProgram;
	std::unique_ptr<ShaderProgram> mTrailMapProgram;
	std::unique_ptr<ShaderProgram> mDecayProgram;
	GLuint                         mAgentBuffer;
	GLuint                         mParameterBuffer;

	GLuint  mTrailMap1;
	GLuint  mTrailMap2;
	GLuint* mTrailMap;
	GLuint* mDiffusedTrailMap;

	float      mSimulationSpeed = 0.0f;
	Parameters mParameters      = {};
};