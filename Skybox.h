#pragma once
#include "GL.h"
#include <mgpcl/String.h>

class Skybox
{
public:
    Skybox();
    ~Skybox();

    bool load(const m::String &fname);
    void draw();

private:
    GLuint m_tex;
    GLuint m_vbo;
    GLuint m_vao;
};
