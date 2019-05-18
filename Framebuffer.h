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
    void createColorBuffer(int idx, gl::TextureFormat tf);
    void createDepthBuffer();
    bool finishFramebuffer();

    void bindForRender()
    {
        gl::bindFramebuffer(gl::kFBT_DrawFramebuffer, m_fbo);
        gl::viewport(0, 0, static_cast<GLsizei>(m_w), static_cast<GLsizei>(m_h));
    }

    static void unbindFromRender(uint32_t w, uint32_t h)
    {
        gl::bindFramebuffer(gl::kFBT_DrawFramebuffer, 0);
        gl::viewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
    }

    GLuint colorAttachmentID(int idx = 0) const
    {
        return m_color[idx];
    }

    Framebuffer &operator = (Framebuffer &&src);

    uint32_t width() const
    {
        return m_w;
    }

    uint32_t height() const
    {
        return m_h;
    }

private:
    uint32_t m_w;
    uint32_t m_h;
    GLuint m_color[2];
    GLuint m_depth;
    GLuint m_fbo;
};
