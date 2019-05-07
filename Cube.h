#pragma once
#include <cstdint>
#include "GameObject.h"
#include "GL.h"

class Cube : GameObject
{
public:
	Cube();
	virtual ~Cube();
	void update(float dt);
	void renderer(float ptt);
	void drawCube();

private :
	GLuint m_bufferData;
	GLuint m_bufferIndexs;
	GLuint m_vertexArray;
};