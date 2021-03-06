#pragma once

#include "core/Core.h"

class GLMesh;
class MeshData;
struct Vertex;
class ShaderProgram;

// This is probably not a good thing to do... but it makes the three debug modes accessible without requiring to include GLEW
#define POINTS 0x0000
#define LINES 0x0001
#define TRIANGLES 0x0004

class DebugRenderer {
private:
	ShaderProgram* debugShader;
	GLMesh* pointMesh;
	GLMesh* lineMesh;
	GLMesh* triangleMesh;


	fvec4 colour;
	float lineSize;
	float pointSize;
	bool enableDepth;
	bool enableLighting;
	bool enableBlend;

	uint32 blendSrcFactor;
	uint32 blendDstFactor;

	bool drawing;
	MeshData* currMeshData;
	uint32 currentMode;
public:
	DebugRenderer();
	~DebugRenderer();

	void init();

	void setColour(fvec4 colour);

	void setLineSize(float size);

	void setPointSize(float size);

	void setDepthEnabled(bool enabled);

	void setLightingEnabled(bool enabled);

	void setBlendEnabled(bool enabled);

	void setBlend(uint32 sfactor, uint32 dfactor);

	void begin(uint32 mode);

	void draw(std::vector<Vertex> vertices, std::vector<int32> indices, dmat4 modelMatrix = dmat4(1.0));

	void finish();

	void renderMesh(GLMesh* mesh);

	ShaderProgram* getDebugShader() const;
};

