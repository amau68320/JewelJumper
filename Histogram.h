#pragma once
#include <aiso/GL.h>
#include <mgpcl/List.h>

enum
{
    HistogramSize = 64,
    HistogramNumBuffers = 2,
    HistogramWorkgroupSize = 16,
    HistogramNumShaders = 3
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
        return static_cast<float>(m_histo[idx]) / static_cast<float>(m_ww * m_wh);
    }

private:
    bool loadShader(int id, const char *fname);

    GLuint m_shader[HistogramNumShaders];
    GLuint m_program[HistogramNumShaders];

    GLuint m_ssbo[HistogramNumBuffers];
    int m_curBuf;

    m::List<GLuint> m_interTexs;

    GLuint *m_histo;
    GLuint m_ww;
    GLuint m_wh;
    GLuint m_workgroupsX;
    GLuint m_workgroupsY;
};
