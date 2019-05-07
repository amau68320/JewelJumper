#include "Shader.h"
#include <mgpcl/FileIOStream.h>
#include <mgpcl/StringIOStream.h>
#include <mgpcl/Logger.h>

Shader::Shader()
{
    m_vert = 0;
    m_geom = 0;
    m_frag = 0;
    m_prog = 0;
}

Shader::Shader(Shader &&src) : m_log(std::move(src.m_log))
{
    m_vert = src.m_vert;
    m_geom = src.m_geom;
    m_frag = src.m_frag;
    m_prog = src.m_prog;
    src.m_vert = 0;
    src.m_geom = 0;
    src.m_frag = 0;
    src.m_prog = 0;
}

Shader::~Shader()
{
    destroy();
}

bool Shader::load(const m::String &vert, const m::String &frag)
{
    return load(vert, "", frag);
}

bool Shader::load(const m::String &vert, const m::String &geom, const m::String &frag)
{
    destroy();

    m_vert = loadShader(vert, gl::kST_VertexShader);
    if(m_vert == 0)
        return false;

    if(!geom.isEmpty()) {
        m_geom = loadShader(geom, gl::kST_GeometryShader);
        if(m_geom == 0)
            return false;
    }

    m_frag = loadShader(frag, gl::kST_FragmentShader);
    if(m_frag == 0)
        return false;

    m_prog = gl::createProgram();
    if(m_prog == 0) {
        m_log = "gl::createProgram() failed!";
        return false;
    }

    gl::attachShader(m_prog, m_vert);
    if(!geom.isEmpty())
        gl::attachShader(m_prog, m_geom);

    gl::attachShader(m_prog, m_frag);
    gl::linkProgram(m_prog);

    GLint status;
    gl::getProgramiv(m_prog, gl::kPP_LinkStatus, &status);

    if(status != GL_TRUE) {
        GLint size;
        gl::getProgramiv(m_prog, gl::kPP_InfoLogLength, &size);

        m_log.cleanup();
        m_log.append(' ', size);
        gl::getProgramInfoLog(m_prog, size, &size, m_log.begin());
        return false;
    }

    return true;
}

static const char *shaderTypeName(gl::ShaderType type)
{
    switch(type) {
    case gl::kST_VertexShader  : return "vertex";
    case gl::kST_GeometryShader: return "geometry";
    case gl::kST_FragmentShader: return "fragment";
    default: return "wtf";
    }
}

GLuint Shader::loadShader(const m::String &file, gl::ShaderType type)
{
    mlogger.debug(M_LOG, "Loading %s shader \"%s\"...", shaderTypeName(type), file.raw());

    m::FileInputStream fis;
    m::FileInputStream::OpenError err = fis.open(file);
    if(err == m::FileInputStream::kOE_FileNotFound) {
        m_log = "Shader source \"";
        m_log += file;
        m_log += "\" could not be found!";
        return 0;
    } else if(err != m::FileInputStream::kOE_Success) {
        m_log = "Could not open shader source \"";
        m_log += file;
        m_log += '"';
        return 0;
    }

    m::StringOStream sos;
    if(!m::IO::transfer(&sos, &fis)) {
        m_log = "Could not read shader source \"";
        m_log += file;
        m_log += '"';
        return 0;
    }

    GLuint ret = gl::createShader(type);
    if(ret == 0) {
        m_log = "gl::createShader() failed!";
        return 0;
    }

    const GLchar *src = sos.data().raw();
    const int len = sos.data().length();
    gl::shaderSource(ret, 1, &src, &len);
    gl::compileShader(ret);

    GLint status;
    gl::getShaderiv(ret, gl::kSP_CompileStatus, &status);

    if(status != GL_TRUE) {
        GLint size;
        gl::getShaderiv(ret, gl::kSP_InfoLogLength, &size);

        m_log.cleanup();
        m_log.append(' ', size);
        gl::getShaderInfoLog(ret, size, &size, m_log.begin());
        gl::deleteShader(ret);
        return 0;
    }

    return ret;
}

Shader &Shader::operator = (Shader &&src)
{
    destroy();

    m_vert = src.m_vert;
    m_geom = src.m_geom;
    m_frag = src.m_frag;
    m_prog = src.m_prog;
    src.m_vert = 0;
    src.m_geom = 0;
    src.m_frag = 0;
    src.m_prog = 0;
    m_log = std::move(src.m_log);
    return *this;
}

void Shader::destroy()
{
    if(m_prog != 0)
        gl::deleteProgram(m_prog);

    if(m_frag != 0)
        gl::deleteShader(m_frag);

    if(m_geom != 0)
        gl::deleteShader(m_geom);

    if(m_vert != 0)
        gl::deleteShader(m_vert);
}
