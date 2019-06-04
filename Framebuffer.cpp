#include "Framebuffer.h"
#include <mgpcl/Assert.h>

Framebuffer::Framebuffer() : m_w(0), m_h(0), m_color{ 0, 0 }, m_depth(0), m_fbo(0), m_depthMode(kFDM_DepthStencilRenderbuffer)
{
}

Framebuffer::Framebuffer(uint32_t w, uint32_t h) : m_w(w), m_h(h), m_color{ 0, 0 }, m_depth(0), m_fbo(0), m_depthMode(kFDM_DepthStencilRenderbuffer)
{
}

Framebuffer::Framebuffer(Framebuffer &&src) : m_w(src.m_w), m_h(src.m_h), m_color{ src.m_color[0], src.m_color[1] }, m_depth(src.m_depth), m_fbo(src.m_fbo), m_depthMode(src.m_depthMode)
{
    src.m_color[0] = 0;
    src.m_color[1] = 0;
    src.m_depth    = 0;
    src.m_fbo      = 0;
}

Framebuffer::~Framebuffer()
{
    if(m_fbo != 0)
        gl::deleteFramebuffer(m_fbo);

    if(m_depth != 0) {
        if(m_depthMode == kFDM_DepthTexture)
            gl::deleteTexture(m_depth);
        else
            gl::deleteRenderbuffer(m_depth);
    }

    if(m_color[0] != 0)
        gl::deleteTexture(m_color[0]);

    if(m_color[1] != 0)
        gl::deleteTexture(m_color[1]);
}

void Framebuffer::init(uint32_t w, uint32_t h)
{
    m_w = w;
    m_h = h;
}

void Framebuffer::createColorBuffer(int idx, gl::TextureFormat tf)
{
    mAssert(m_w != 0 && m_h != 0, "createColorBuffer() appele avant init()");
    if(m_color[idx] != 0)
        gl::deleteTexture(m_color[idx]);

    m_color[idx] = gl::genTexture();
    gl::bindTexture(gl::kTT_Texture2D, m_color[idx]);
    gl::texStorage2D(gl::kTT_Texture2D, 1, tf, m_w, m_h);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MagFilter, gl::kTF_Linear);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MinFilter, gl::kTF_Linear);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_WrapS, gl::kTWM_ClampToEdge);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_WrapT, gl::kTWM_ClampToEdge);
    gl::bindTexture(gl::kTT_Texture2D, 0);
}

void Framebuffer::createDepthBuffer(FramebufferDepthMode mode)
{
    mAssert(m_w != 0 && m_h != 0, "createDepthBuffer() appele avant init()");
    if(m_depth != 0) {
        if(m_depthMode == kFDM_DepthTexture)
            gl::deleteTexture(m_depth);
        else
            gl::deleteRenderbuffer(m_depth);
    }

    m_depthMode = mode;

    if(mode == kFDM_DepthTexture) {
        m_depth = gl::genTexture();
        gl::bindTexture(gl::kTT_Texture2D, m_depth);
        gl::texStorage2D(gl::kTT_Texture2D, 1, gl::kTF_DepthComponent24, m_w, m_h);
        gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MagFilter, gl::kTF_Linear);
        gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MinFilter, gl::kTF_Linear);
        gl::texParameteri(gl::kTT_Texture2D, gl::kTP_WrapS, gl::kTWM_ClampToEdge);
        gl::texParameteri(gl::kTT_Texture2D, gl::kTP_WrapT, gl::kTWM_ClampToEdge);
        gl::bindTexture(gl::kTT_Texture2D, 0);
    } else {
        m_depth = gl::genRenderbuffer();
        gl::bindRenderbufffer(gl::kRT_Renderbuffer, m_depth);
        gl::renderbufferStorage(gl::kRT_Renderbuffer, (mode == kFDM_DepthRenderbuffer) ? gl::kRF_DepthComponent24 : gl::kRF_Depth24Stencil8, m_w, m_h);
        gl::bindRenderbufffer(gl::kRT_Renderbuffer, 0);
    }
}

bool Framebuffer::finishFramebuffer()
{
    if(m_fbo != 0)
        gl::deleteFramebuffer(m_fbo);

    m_fbo = gl::genFramebuffer();
    gl::bindFramebuffer(gl::kFBT_Framebuffer, m_fbo);

    if(m_color[0] != 0)
        gl::framebufferTexture2D(gl::kFBT_Framebuffer, gl::kFBA_ColorAttachment0, gl::kTT_Texture2D, m_color[0], 0);

    if(m_color[1] != 0)
        gl::framebufferTexture2D(gl::kFBT_Framebuffer, gl::kFBA_ColorAttachment1, gl::kTT_Texture2D, m_color[1], 0);

    if(m_depth != 0) {
        switch(m_depthMode) {
        case kFDM_DepthStencilRenderbuffer: gl::framebufferRenderbuffer(gl::kFBT_Framebuffer, gl::kFBA_DepthStencilAttachment, gl::kRT_Renderbuffer, m_depth); break;
        case kFDM_DepthRenderbuffer:        gl::framebufferRenderbuffer(gl::kFBT_Framebuffer, gl::kFBA_DepthAttachment, gl::kRT_Renderbuffer, m_depth);        break;
        case kFDM_DepthTexture:             gl::framebufferTexture2D(gl::kFBT_Framebuffer, gl::kFBA_DepthAttachment, gl::kTT_Texture2D, m_depth, 0);           break;
        }
    }

    bool ret = gl::checkFramebufferStatus(gl::kFBT_Framebuffer) == gl::kFBS_Complete;
    gl::bindFramebuffer(gl::kFBT_Framebuffer, 0);
    return ret;
}

Framebuffer &Framebuffer::operator = (Framebuffer &&src)
{
    if(m_fbo != 0)
        gl::deleteFramebuffer(m_fbo);

    if(m_depth != 0) {
        if(m_depthMode == kFDM_DepthTexture)
            gl::deleteTexture(m_depth);
        else
            gl::deleteRenderbuffer(m_depth);
    }

    if(m_color[0] != 0)
        gl::deleteTexture(m_color[0]);

    if(m_color[1] != 0)
        gl::deleteTexture(m_color[1]);

    m_w         = src.m_w;
    m_h         = src.m_h;
    m_color[0]  = src.m_color[0];
    m_color[1]  = src.m_color[1];
    m_depth     = src.m_depth;
    m_fbo       = src.m_fbo;
    m_depthMode = src.m_depthMode;

    src.m_color[0] = 0;
    src.m_color[1] = 0;
    src.m_depth    = 0;
    src.m_fbo      = 0;

    return *this;
}
