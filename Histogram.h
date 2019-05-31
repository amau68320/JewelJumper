#pragma once
#include <aiso/GL.h>

enum
{
    HistogramSize = 64,
    HistogramNumBuffers = 4,
    HistogramWorkgroupSize = 16
};

class Histogram
{
public:
    Histogram();
    ~Histogram();

    bool setup(GLuint w, GLuint h);
    void compute(GLuint color);

    float value(int idx) const
    {
        return static_cast<float>(m_histo[idx]) / static_cast<float>(m_ww * m_wh * HistogramWorkgroupSize * HistogramWorkgroupSize);
    }

private:
    GLuint m_shader;
    GLuint m_program;
    GLuint m_ssbo[HistogramNumBuffers];
    int m_curBuf;
    GLuint *m_histo;
    GLuint m_ww;
    GLuint m_wh;
};
