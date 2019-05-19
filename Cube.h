#pragma once
#include <cstdint>
#include "GameObject.h"
#include <aiso/GL.h>

class Cube : public GameObject
{
public:
	Cube();
	virtual ~Cube();
	void update(float dt);
	void render(float ptt);

private :
	GLuint m_bufferData;
	GLuint m_bufferIndexs;
	GLuint m_vertexArray;
};