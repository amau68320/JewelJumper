#include "Histogram.h"
#include <mgpcl/FileIOStream.h>
#include <mgpcl/StringIOStream.h>
#include <mgpcl/Logger.h>

Histogram::Histogram() : m_curBuf(0), m_ww(0), m_wh(0)
{
    m::mem::zero(m_shader);
    m::mem::zero(m_program);
    m::mem::zero(m_ssbo);
    m_histo = new GLuint[HistogramSize];
}

Histogram::~Histogram()
{
    for(int i = 0; i < HistogramNumBuffers; i++) {
        if(m_ssbo[i] != 0)
            gl::deleteBuffer(m_ssbo[i]);
    }

    for(ITex &tex : m_interTexs)
        gl::deleteTexture(tex.id);

    for(int i = 0; i < HistogramNumShaders; i++) {
        if(m_program[i] != 0)
            gl::deleteProgram(m_program[i]);

        if(m_shader[i] != 0)
            gl::deleteShader(m_shader[i]);
    }

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

static GLuint nearestPower(GLuint x)
{
    GLuint ret = 1;
    while(ret < x)
        ret <<= 1;

    return ret;
}

bool Histogram::setup(GLuint w, GLuint h)
{
    m_ww = w;
    m_wh = h;

    if(!loadShader(0, "shaders/histogram_1.comp"))
        return false;

    if(!loadShader(1, "shaders/histogram_2.comp"))
        return false;

    if(!loadShader(2, "shaders/histogram_3.comp"))
        return false;
    
    //Buffers de sortie
    for(int i = 0; i < HistogramNumBuffers; i++) {
        m_ssbo[i] = gl::genBuffer();
        gl::bindBuffer(gl::kBT_ShaderStorageBuffer, m_ssbo[i]);
        gl::bufferData(gl::kBT_ShaderStorageBuffer, HistogramSize * sizeof(GLuint), nullptr, gl::kBU_DynamicCopy);
        gl::bindBuffer(gl::kBT_ShaderStorageBuffer, 0);
    }

    GLuint p2w = nearestPower(w) >> 1;
    GLuint p2h = nearestPower(h) >> 1;

    while(p2w > 2 || p2h > 2) {
        if(p2w > 2)
            p2w >>= 1;

        if(p2h > 2)
            p2h >>= 1;

        GLuint tex = gl::genTexture();
        gl::bindTexture(gl::kTT_Texture3D, tex);
        gl::texStorage3D(gl::kTT_Texture3D, 1, gl::kTF_R32UI, p2w, p2h, HistogramSize);
        gl::texParameteri(gl::kTT_Texture3D, gl::kTP_MagFilter, gl::kTF_Nearest);
        gl::texParameteri(gl::kTT_Texture3D, gl::kTP_MinFilter, gl::kTF_Nearest);
        gl::texParameteri(gl::kTT_Texture3D, gl::kTP_WrapR, gl::kTWM_ClampToEdge);
        gl::texParameteri(gl::kTT_Texture3D, gl::kTP_WrapS, gl::kTWM_ClampToEdge);
        gl::texParameteri(gl::kTT_Texture3D, gl::kTP_WrapT, gl::kTWM_ClampToEdge);

        gl::bindTexture(gl::kTT_Texture3D, 0);
        m_interTexs << ITex(tex, p2w, p2h);
    }

    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
        mlogger.error(M_LOG, "Histogram::setup(): OpenGL error %x", err);

    return true;
}

void Histogram::compute(GLuint color)
{
    //Effacer l'ancien buffer
    gl::bindBuffer(gl::kBT_ShaderStorageBuffer, m_ssbo[m_curBuf]);
    void *ptr = gl::mapBuffer(gl::kBT_ShaderStorageBuffer, gl::kBA_ReadOnly);
    m::mem::copy(m_histo, ptr, HistogramSize * sizeof(GLuint));
    gl::unmapBuffer(gl::kBT_ShaderStorageBuffer);
    gl::bindBuffer(gl::kBT_ShaderStorageBuffer, 0);
    m_curBuf = (m_curBuf + 1) % HistogramNumBuffers;

    //Passe 1
    gl::useProgram(m_program[0]);
    gl::memoryBarrier(gl::kMBF_ShaderImageAccess);
    gl::bindImageTexture(0, color, 0, false, 0, gl::kBA_ReadOnly, gl::kTF_RGBA16F);
    gl::bindImageTexture(1, m_interTexs[0].id, 0, false, 0, gl::kBA_ReadWrite, gl::kTF_R32UI);
    gl::dispatchCompute(m_interTexs[0].workgroupsX(), m_interTexs[0].workgroupsY(), 1);

    //Passe 2
    gl::useProgram(m_program[1]);
    for(int i = 1; i < ~m_interTexs; i++) {
        const ITex &dst = m_interTexs[i];

        gl::memoryBarrier(gl::kMBF_ShaderImageAccess);
        gl::bindImageTexture(0, m_interTexs[i - 1].id, 0, false, 0, gl::kBA_ReadOnly, gl::kTF_R32UI);
        gl::bindImageTexture(1, dst.id, 0, false, 0, gl::kBA_WriteOnly, gl::kTF_R32UI);
        gl::dispatchCompute(dst.workgroupsX(), dst.workgroupsY(), 1);
    }

    //Passe 3
    gl::useProgram(m_program[2]);
    gl::memoryBarrier(gl::kMBF_ShaderImageAccess);
    gl::bindImageTexture(0, m_interTexs.last().id, 0, false, 0, gl::kBA_ReadOnly, gl::kTF_R32UI);
    gl::bindBufferBase(gl::kBT_ShaderStorageBuffer, 1, m_ssbo[m_curBuf]);
    gl::dispatchCompute(1, 1, 1);
    gl::useProgram(0);

    //Verif. erreurs
    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
        mlogger.error(M_LOG, "Histogram::compute(): OpenGL error %x", err);

    /*GLuint histoSum = 0;
    for(int i = 0; i < HistogramSize; i++)
        histoSum += m_histo[i];

    int diff = static_cast<int>(histoSum) - static_cast<int>(m_ww * m_wh);
    if(diff != 0)
        mlogger.error(M_LOG, "Not good: %d", diff);*/
}

bool Histogram::loadShader(int id, const char *fname)
{
    //On compile le shader
    m::String src;
    if(!readFile(fname, src)) {
        mlogger.error(M_LOG, "Erreur lors du chargement du shader de calcul d'histogramme %d...", id + 1);
        return false;
    }

    const GLchar *pSrc = src.raw();
    int len = src.length();

    m_shader[id] = gl::createShader(gl::kST_ComputeShader);
    gl::shaderSource(m_shader[id], 1, &pSrc, &len);
    gl::compileShader(m_shader[id]);

    GLint status;
    gl::getShaderiv(m_shader[id], gl::kSP_CompileStatus, &status);

    if(status != GL_TRUE) {
        GLint size;
        gl::getShaderiv(m_shader[id], gl::kSP_InfoLogLength, &size);

        src.cleanup();
        src.append(' ', size); //HAX
        gl::getShaderInfoLog(m_shader[id], size, &size, src.begin());
        gl::deleteShader(m_shader[id]);
        m_shader[id] = 0;

        mlogger.error(M_LOG, "Erreur lors de la compilation du shader d'histogramme %d:", id + 1);
        mlogger.error(M_LOG, "%s", src.raw());
        return false;
    }

    //On cree le programme
    m_program[id] = gl::createProgram();
    gl::attachShader(m_program[id], m_shader[id]);
    gl::linkProgram(m_program[id]);
    gl::getProgramiv(m_program[id], gl::kPP_LinkStatus, &status);

    if(status != GL_TRUE) {
        GLint size;
        gl::getProgramiv(m_program[id], gl::kPP_InfoLogLength, &size);

        src.cleanup();
        src.append(' ', size);
        gl::getProgramInfoLog(m_program[id], size, &size, src.begin());
        gl::deleteProgram(m_program[id]);
        m_program[id] = 0;

        mlogger.error(M_LOG, "Erreur lors de l'edition des liens du programme d'histogramme %d:", id + 1);
        mlogger.error(M_LOG, "%s", src.raw());
        return false;
    }

    return true;
}
