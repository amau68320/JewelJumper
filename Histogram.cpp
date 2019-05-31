#include "Histogram.h"
#include <mgpcl/FileIOStream.h>
#include <mgpcl/StringIOStream.h>
#include <mgpcl/Logger.h>

Histogram::Histogram() : m_shader(0), m_program(0), m_curBuf(0), m_ww(0), m_wh(0)
{
    m::mem::zero(m_ssbo);
    m_histo = new GLuint[HistogramSize];
}

Histogram::~Histogram()
{
    for(int i = 0; i < HistogramNumBuffers; i++) {
        if(m_ssbo[i] != 0)
            gl::deleteBuffer(m_ssbo[i]);
    }

    if(m_program != 0)
        gl::deleteProgram(m_program);

    if(m_shader != 0)
        gl::deleteShader(m_shader);

    delete[] m_histo;
}

static bool readFile(const char *path, m::String &dst)
{
    m::FileInputStream fis;

    if(fis.open(path) != m::FileInputStream::kOE_Success)
        return false;

    m::StringOStream sos;
    if(!m::IO::transfer(&sos, &fis, 8192))
        return false;

    fis.close();
    dst = sos.data();
    return true;
}

bool Histogram::setup(GLuint w, GLuint h)
{
    m_ww = w / HistogramWorkgroupSize;
    m_wh = h / HistogramWorkgroupSize;

    //On compile le shader
    m::String src;
    if(!readFile("shaders/histogram.comp", src)) {
        mlogger.error(M_LOG, "Erreur lors du chargement du shader de calcul d'histogramme...");
        return false;
    }

    const GLchar *pSrc = src.raw();
    int len = src.length();

    m_shader = gl::createShader(gl::kST_ComputeShader);
    gl::shaderSource(m_shader, 1, &pSrc, &len);
    gl::compileShader(m_shader);

    GLint status;
    gl::getShaderiv(m_shader, gl::kSP_CompileStatus, &status);

    if(status != GL_TRUE) {
        GLint size;
        gl::getShaderiv(m_shader, gl::kSP_InfoLogLength, &size);

        src.cleanup();
        src.append(' ', size); //HAX
        gl::getShaderInfoLog(m_shader, size, &size, src.begin());
        gl::deleteShader(m_shader);
        m_shader = 0;

        mlogger.error(M_LOG, "Erreur lors de la compilation du shader d'histogramme:");
        mlogger.error(M_LOG, "%s", src.raw());
        return false;
    }

    //On cree le programme
    m_program = gl::createProgram();
    gl::attachShader(m_program, m_shader);
    gl::linkProgram(m_program);
    gl::getProgramiv(m_program, gl::kPP_LinkStatus, &status);

    if(status != GL_TRUE) {
        GLint size;
        gl::getProgramiv(m_program, gl::kPP_InfoLogLength, &size);

        src.cleanup();
        src.append(' ', size);
        gl::getProgramInfoLog(m_program, size, &size, src.begin());

        mlogger.error(M_LOG, "Erreur lors de l'edition des liens du programme d'histogramme:");
        mlogger.error(M_LOG, "%s", src.raw());
        return false;
    }

    //Et pour finir, les buffers
    for(int i = 0; i < HistogramNumBuffers; i++) {
        m_ssbo[i] = gl::genBuffer();
        gl::bindBuffer(gl::kBT_ShaderStorageBuffer, m_ssbo[i]);
        gl::bufferData(gl::kBT_ShaderStorageBuffer, HistogramSize * sizeof(GLuint), nullptr, gl::kBU_StaticDraw);
        gl::bindBuffer(gl::kBT_ShaderStorageBuffer, 0);
    }

    return true;
}

void Histogram::compute(GLuint color)
{
    //Effacer l'ancien buffer
    gl::bindBuffer(gl::kBT_ShaderStorageBuffer, m_ssbo[m_curBuf]);
    void *ptr = gl::mapBuffer(gl::kBT_ShaderStorageBuffer, gl::kBA_ReadWrite);
    m::mem::copy(m_histo, ptr, HistogramSize * sizeof(GLuint));
    m::mem::zero(ptr, HistogramSize * sizeof(GLuint));
    gl::unmapBuffer(gl::kBT_ShaderStorageBuffer);
    gl::bindBuffer(gl::kBT_ShaderStorageBuffer, 0);
    m_curBuf = (m_curBuf + 1) % HistogramNumBuffers;

    gl::useProgram(m_program);
    gl::bindImageTexture(0, color, 0, false, 0, gl::kBA_ReadOnly, gl::kTF_RGBA16F);
    gl::bindBufferBase(gl::kBT_ShaderStorageBuffer, 1, m_ssbo[m_curBuf]);
    gl::dispatchCompute(m_ww, m_wh, 1);
    gl::useProgram(0);

    GLenum test = glGetError();
    if(test != GL_NO_ERROR)
        mlogger.error(M_LOG, "OpenGL error %d", test);
}
