#include "Histogram.h"
#include <mgpcl/FileIOStream.h>
#include <mgpcl/StringIOStream.h>
#include <mgpcl/Logger.h>
#include <mgpcl/Math.h>
#include <mgpcl/Time.h>

Histogram::Histogram() : m_curBuf(0), m_ww(0), m_wh(0), m_oldAutoExposure(1.0f), m_autoExposure(1.0f), m_temporalL(0.0f),
                         m_tau(4.0f), m_dispatchPos(0), m_dispatchMarks{ 1, 2, 3, 4 }, m_dispatchCount(4)
{
    m::mem::zero(m_shader);
    m::mem::zero(m_program);
    m::mem::zero(m_ssbo);

    m_histo1 = new GLuint[HistogramSize];
    m_histo2 = new GLuint[HistogramSize];
    m_lastTime = m::time::getTimeMs();
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

    delete[] m_histo1;
    delete[] m_histo2;
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

    m_dispatchCount = 4;
    m_dispatchMarks[0] = 1;
    m_dispatchMarks[1] = (m_interTexs.size() * 4) / 10;
    m_dispatchMarks[2] = (m_interTexs.size() * 7) / 10;
    m_dispatchMarks[3] = m_interTexs.size();

    for(int i = 0; i < 4; i++)
        mlogger.debug(M_LOG, "m_dispatchMarks[%d] = %d", i, m_dispatchMarks[i]);

    if(m_dispatchMarks[1] == 1 || m_dispatchMarks[2] == m_dispatchMarks[1] || m_dispatchMarks[3] == m_dispatchMarks[2]) {
        m_dispatchCount = 2;
        m_dispatchMarks[1] = m_interTexs.size();
    }

    mlogger.debug(M_LOG, "Nombre de dispatch: %d", m_dispatchCount);

    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
        mlogger.error(M_LOG, "Histogram::setup(): Erreur OpenGL %x", err);

    return true;
}

void Histogram::compute(GLuint color)
{
    if(m_dispatchPos == 0) {
        //Copier l'ancien buffer
        gl::bindBuffer(gl::kBT_ShaderStorageBuffer, m_ssbo[m_curBuf]);
        void *ptr = gl::mapBuffer(gl::kBT_ShaderStorageBuffer, gl::kBA_ReadOnly);
        m::mem::copy(m_histo1, ptr, HistogramSize * sizeof(GLuint));
        gl::unmapBuffer(gl::kBT_ShaderStorageBuffer);
        gl::bindBuffer(gl::kBT_ShaderStorageBuffer, 0);
        m_curBuf = (m_curBuf + 1) % HistogramNumBuffers;

        //Passe 1
        gl::useProgram(m_program[0]);
        gl::memoryBarrier(gl::kMBF_ShaderImageAccess);
        gl::bindImageTexture(0, color, 0, false, 0, gl::kBA_ReadOnly, gl::kTF_RGBA16F);
        gl::bindImageTexture(1, m_interTexs[0].id, 0, true, 0, gl::kBA_ReadWrite, gl::kTF_R32UI);
        gl::dispatchCompute(m_interTexs[0].workgroupsX(), m_interTexs[0].workgroupsY(), 1);
        gl::useProgram(0);

        m_dispatchPos++;
    } else {
        //Passe 2
        gl::useProgram(m_program[1]);

        for(int i = m_dispatchMarks[m_dispatchPos - 1]; i < m_dispatchMarks[m_dispatchPos]; i++) {
            const ITex &dst = m_interTexs[i];

            gl::memoryBarrier(gl::kMBF_ShaderImageAccess);
            gl::bindImageTexture(0, m_interTexs[i - 1].id, 0, true, 0, gl::kBA_ReadOnly, gl::kTF_R32UI);
            gl::bindImageTexture(1, dst.id, 0, true, 0, gl::kBA_WriteOnly, gl::kTF_R32UI);
            gl::dispatchCompute(dst.workgroupsX(), dst.workgroupsY(), 1);
        }

        if(++m_dispatchPos >= m_dispatchCount) {
            //Passe 3
            gl::useProgram(m_program[2]);
            gl::memoryBarrier(gl::kMBF_ShaderImageAccess);
            gl::bindImageTexture(0, m_interTexs.last().id, 0, true, 0, gl::kBA_ReadOnly, gl::kTF_R32UI);
            gl::bindBufferBase(gl::kBT_ShaderStorageBuffer, 1, m_ssbo[m_curBuf]);
            gl::dispatchCompute(1, 1, 1);

            m_dispatchPos = 0;
        }

        gl::useProgram(0);
    }

    //Verif. erreurs
    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
        mlogger.error(M_LOG, "Histogram::compute(): Erreur OpenGL %x", err);

    if(m_dispatchPos == 0) {
        //Calcul expo. auto
        m::mem::copy(m_histo2, m_histo1, HistogramSize * sizeof(GLuint));
        GLuint amntDark = (m_ww * m_wh * 10) / 100;
        GLuint amntBright = (m_ww * m_wh * 20) / 100;
        int calcStart = 0;
        int calcEnd = HistogramSize - 1;

        for(int i = 0; i < HistogramSize; i++) {
            if(amntDark > m_histo2[i]) {
                amntDark -= m_histo2[i];
                m_histo2[i] = 0;
            } else {
                m_histo2[i] -= amntDark;
                calcStart = i;
                break;
            }
        }

        for(int i = HistogramSize - 1; i >= calcStart; i--) {
            if(amntBright > m_histo2[i]) {
                amntBright -= m_histo2[i];
                m_histo2[i] = 0;
            } else {
                m_histo2[i] -= amntBright;
                calcEnd = i;
                break;
            }
        }

        float avgL = 0.0f;
        GLuint count = 0;

        for(int i = calcStart; i <= calcEnd; i++) {
            const float ev100 = static_cast<float>(i) / 1.96875f - 16.0f;
            const float L = std::exp2(ev100) * 0.125f;

            avgL += L * static_cast<float>(m_histo2[i]);
            count += m_histo2[i];
        }

        avgL /= static_cast<float>(count);

        if(std::isnan(avgL) || avgL < -1000.0f || avgL > 10000.0f)
            return; //Cas rares au debut

        const double t = m::time::getTimeMs();
        const float dt = static_cast<float>((t - m_lastTime) / 1000.0);

        m_lastTime = t;
        m_temporalL = m_temporalL + (avgL - m_temporalL) * (1.0f - std::exp(-dt * m_tau));

        const float keyValue = 1.2f - 2.0f / (std::log10(m_temporalL + 1.0f) + 2.0f);
        m_oldAutoExposure = m_autoExposure;
        m_autoExposure = keyValue / m::math::clamp(m_temporalL, 0.001f, 10.0f);
    }
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
