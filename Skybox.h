#pragma once
#include <aiso/GL.h>
#include <mgpcl/String.h>

class Skybox
{
public:
    Skybox();
    ~Skybox();

    bool load(const m::String &fname);
    void draw();

    void bindCubeMap()
    {
        gl::bindTexture(gl::kTT_TextureCubeMap, m_cubemap);
    }

    static void unbindCubeMap()
    {
        gl::bindTexture(gl::kTT_TextureCubeMap, 0);
    }

private:
    GLuint m_tex;
    GLuint m_vbo;
    GLuint m_vao;
    GLuint m_cubemap;
};
