#pragma once
#include <aiso/GL.h>
#include <mgpcl/String.h>
#include <mgpcl/Thread.h>
#include <mgpcl/Atomic.h>
#include <cstdio>

class Skybox
{
public:
    Skybox();
    ~Skybox();

    bool load(const m::String &fname);
    void draw();
    bool loadAsync(const m::String &fname);
    bool update();

    void bindCubeMap()
    {
        gl::bindTexture(gl::kTT_TextureCubeMap, m_cubemap);
    }

    static void unbindCubeMap()
    {
        gl::bindTexture(gl::kTT_TextureCubeMap, 0);
    }

    bool isDoingAsyncOp() const
    {
        return m_asyncOpInProgress;
    }

private:
    bool loadBlocking(FILE *fp);
    void uploadTextureData();

    GLuint m_tex;
    GLuint m_vbo;
    GLuint m_vao;
    GLuint m_cubemap;

    //Asynchrone
    int m_asyncW;
    int m_asyncH;
    float *m_skyboxData;
    float *m_cubemapData;
    bool m_asyncOpInProgress;
    m::Atomic m_asyncOpFinished;
    m::FunctionalThread *m_thread;
};
