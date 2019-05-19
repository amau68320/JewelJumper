#pragma once

#include <mgpcl/String.h>
#include "GL.h"

class Shader
{
    M_NON_COPYABLE(Shader)

public:
    Shader();
    Shader(Shader &&src);
    ~Shader();

    bool load(const m::String &vert, const m::String &frag);
    bool load(const m::String &vert, const m::String &geom, const m::String &frag);
    Shader &operator = (Shader &&src);

    GLint getUniformLocation(const char *var) const
    {
        return gl::getUniformLocation(m_prog, var);
    }

    GLint getUniformLocation(const m::String &var) const
    {
        return gl::getUniformLocation(m_prog, var.raw());
    }

    const m::String &errorString() const
    {
        return m_log;
    }

    bool isValid() const
    {
        return m_prog != 0;
    }

    GLint id() const
    {
        return m_prog;
    }

    void bind() const
    {
        gl::useProgram(m_prog);
    }

    static void unbind()
    {
        gl::useProgram(0);
    }

private:
    void destroy();
    GLuint loadShader(const m::String &file, gl::ShaderType type);

    GLuint m_vert;
    GLuint m_geom;
    GLuint m_frag;
    GLuint m_prog;
    m::String m_log;
};
