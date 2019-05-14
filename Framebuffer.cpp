#include "Framebuffer.h"
#include <mgpcl/Assert.h>

Framebuffer::Framebuffer() : m_w(0), m_h(0), m_color(0), m_depth(0), m_fbo(0)
{
}

Framebuffer::Framebuffer(uint32_t w, uint32_t h) : m_w(w), m_h(h), m_color(0), m_depth(0), m_fbo(0)
{
}

Framebuffer::Framebuffer(Framebuffer &&src) : m_w(src.m_w), m_h(src.m_h), m_color(src.m_color), m_depth(src.m_depth), m_fbo(src.m_fbo)
{
    src.m_color = 0;
    src.m_depth = 0;
    src.m_fbo = 0;
}

Framebuffer::~Framebuffer()
{
    if(m_fbo != 0)
        gl::deleteFramebuffer(m_fbo);

    if(m_depth != 0)
        gl::deleteRenderbuffer(m_depth);

    if(m_color != 0)
        gl::deleteTexture(m_color);
}

void Framebuffer::init(uint32_t w, uint32_t h)
{
    m_w = w;
    m_h = h;
}

void Framebuffer::createColorBuffer(gl::TextureFormat tf)
{
    mAssert(m_w != 0 && m_h != 0, "createColorBuffer() called before init()");
    if(m_color != 0)
        gl::deleteTexture(m_color);

    m_color = gl::genTexture();
    gl::bindTexture(gl::kTT_Texture2D, m_color);
    gl::texStorage2D(gl::kTT_Texture2D, 1, tf, m_w, m_h);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MagFilter, gl::kTF_Linear);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MinFilter, gl::kTF_Linear);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_WrapS, gl::kTWM_ClampToEdge);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_WrapT, gl::kTWM_ClampToEdge);
    gl::bindTexture(gl::kTT_Texture2D, 0);
}

void Framebuffer::createDepthBuffer()
{
    mAssert(m_w != 0 && m_h != 0, "createDepthBuffer() called before init()");
    if(m_depth != 0)
        gl::deleteRenderbuffer(m_depth);

    m_depth = gl::genRenderbuffer();
    gl::bindRenderbufffer(gl::kRT_Renderbuffer, m_depth);
    gl::renderbufferStorage(gl::kRT_Renderbuffer, gl::kRF_Depth24Stencil8, m_w, m_h);
    gl::bindRenderbufffer(gl::kRT_Renderbuffer, 0);
}

bool Framebuffer::finishFramebuffer()
{
    if(m_fbo != 0)
        gl::deleteFramebuffer(m_fbo);

    m_fbo = gl::genFramebuffer();
    gl::bindFramebuffer(gl::kFBT_Framebuffer, m_fbo);

    if(m_color != 0)
        gl::framebufferTexture2D(gl::kFBT_Framebuffer, gl::kFBA_ColorAttachment0, gl::kTT_Texture2D, m_color, 0);

    if(m_depth != 0)
        gl::framebufferRenderbuffer(gl::kFBT_Framebuffer, gl::kFBA_DepthStencilAttachment, gl::kRT_Renderbuffer, m_depth);

    bool ret = gl::checkFramebufferStatus(gl::kFBT_Framebuffer) == gl::kFBS_Complete;
    gl::bindFramebuffer(gl::kFBT_Framebuffer, 0);
    return ret;
}

Framebuffer &Framebuffer::operator = (Framebuffer &&src)
{
    if(m_fbo != 0)
        gl::deleteFramebuffer(m_fbo);

    if(m_depth != 0)
        gl::deleteRenderbuffer(m_depth);

    if(m_color != 0)
        gl::deleteTexture(m_color);

    m_w = src.m_w;
    m_h = src.m_h;
    m_color = src.m_color;
    m_depth = src.m_depth;
    m_fbo = src.m_fbo;

    src.m_color = 0;
    src.m_depth = 0;
    src.m_fbo = 0;

    return *this;
}
