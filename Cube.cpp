#include "Cube.h"

float vertices[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
					0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
					0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

float colors[] = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
				  0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                  0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
				  0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

uint16_t indexs[] = {0, 1, 3, 0, 3, 2, 5, 4, 7, 7, 4 ,6, 8, 10, 12, 12, 10, 14, 9, 13, 11,
					 13, 15, 11, 16, 20, 17, 17, 20, 21, 18, 19, 22, 19, 23, 22};

Cube::Cube()
{
	m_bufferData = gl::genBuffer();
	gl::bindBuffer(gl::kBT_ArrayBuffer, m_bufferData);
	gl::bufferData(gl::kBT_ArrayBuffer, 72 * 2 * sizeof(float), nullptr, gl::kBU_StaticDraw);
	gl::bufferSubData(gl::kBT_ArrayBuffer, 0, 72 * sizeof(float), vertices);
	gl::bufferSubData(gl::kBT_ArrayBuffer, 72 * sizeof(float), 72 * sizeof(float), colors);
	gl::bindBuffer(gl::kBT_ArrayBuffer, 0);

	m_bufferIndexs = gl::genBuffer();
	gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_bufferIndexs);
	gl::bufferData(gl::kBT_ElementArrayBuffer, 36 * sizeof(uint16_t), indexs, gl::kBU_StaticDraw);
	gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);

	m_vertexArray = gl::genVertexArray();
	gl::bindVertexArray(m_vertexArray);
	gl::bindBuffer(gl::kBT_ArrayBuffer, m_bufferData);
	gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_bufferIndexs);
	gl::enableVertexAttribArray(0);
	gl::enableVertexAttribArray(1);
	gl::vertexAttribPointer(0, 3, gl::kDT_Float, false, 0, 0);
	gl::vertexAttribPointer(1, 3, gl::kDT_Float, false, 0, reinterpret_cast <void*>(72 * sizeof(float)));
	gl::bindVertexArray(0);
	gl::bindBuffer(gl::kBT_ArrayBuffer, 0);
	gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);
}

Cube::~Cube()
{
	gl::deleteBuffer(m_bufferData);
	gl::deleteBuffer(m_bufferIndexs);
	gl::deleteVertexArray(m_vertexArray);
}

void Cube::drawCube()
{
	gl::bindVertexArray(m_vertexArray);
	gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_bufferIndexs);
	gl::drawElements(gl::kDM_Triangles, 36, gl::kDT_UnsignedShort, nullptr);
	gl::bindVertexArray(0);
	gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);
}