#pragma once
#include <aiso/GL.h>
#include <mgpcl/List.h>

enum
{
    HistogramSize = 64,
    HistogramNumBuffers = 2,
    HistogramWorkgroupSize = 16,
    HistogramNumShaders = 3,
    HistogramNumITexs = 3
};

class Histogram
{
public:
    Histogram();
    ~Histogram();

    bool setup(GLuint w, GLuint h, GLuint histoDiv0);
    void compute(GLuint color);

    float value(int idx) const
    {
        return static_cast<float>(m_histo1[idx]) / static_cast<float>(m_ww * m_wh);
    }

    float computedExposure() const
    {
        float t = static_cast<float>(m_dispatchPos) / static_cast<float>(m_dispatchCount);
        return (1.0f - t) * m_oldAutoExposure + t * m_autoExposure;
    }

    float temporalAdaptationFactor() const
    {
        return m_tau;
    }

    void setTemporalAdaptationFactor(float tau)
    {
        m_tau = tau;
    }

    int dispatchPos() const
    {
        return m_dispatchPos;
    }

    int dispatchCount() const
    {
        return m_dispatchCount;
    }

private:
    class ITex
    {
    public:
        ITex(GLuint _id, GLuint _w, GLuint _h) : id(_id), w(_w), h(_h)
        {
        }

        GLuint workgroupsX() const
        {
            return (w + HistogramWorkgroupSize - 1) / HistogramWorkgroupSize;
        }

        GLuint workgroupsY() const
        {
            return (h + HistogramWorkgroupSize - 1) / HistogramWorkgroupSize;
        }

        GLuint id;
        GLuint w;
        GLuint h;
    };

    bool loadShader(int id, const char *fname, GLuint dispatch0Mult);

    GLuint m_shader[HistogramNumShaders];
    GLuint m_program[HistogramNumShaders];

    GLuint m_ssbo[HistogramNumBuffers];
    int m_curBuf;

    m::List<ITex> m_interTexs;
    GLuint m_texs[HistogramNumITexs];

    GLuint m_histo1[HistogramSize];
    GLuint m_histo2[HistogramSize];
    GLuint m_ww;
    GLuint m_wh;

    float m_oldAutoExposure;
    float m_autoExposure;
    float m_temporalL;
    float m_tau;
    double m_lastTime;

    int m_dispatchPos;
    int m_dispatchMarks[4];
    int m_dispatchCount;
};
