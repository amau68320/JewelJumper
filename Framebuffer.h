#pragma once
#include "GL.h"
#include <cstdint>
#include <mgpcl/Util.h>

class Framebuffer
{
    M_NON_COPYABLE(Framebuffer)

public:
    Framebuffer();
    Framebuffer(uint32_t w, uint32_t h);
    Framebuffer(Framebuffer &&src);
    ~Framebuffer();

    void init(uint32_t w, uint32_t h);
    void createColorBuffer(gl::TextureFormat tf);
    void createDepthBuffer();
    bool finishFramebuffer();

    void bindForRender()
    {
        gl::bindFramebuffer(gl::kFBT_DrawFramebuffer, m_fbo);
    }

    static void unbindFromRender()
    {
        gl::bindFramebuffer(gl::kFBT_DrawFramebuffer, 0);
    }

    GLuint colorAttachmentID() const
    {
        return m_color;
    }

    Framebuffer &operator = (Framebuffer &&src);

private:
    uint32_t m_w;
    uint32_t m_h;
    GLuint m_color;
    GLuint m_depth;
    GLuint m_fbo;
};
