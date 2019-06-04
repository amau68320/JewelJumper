#pragma once
#include <aiso/GL.h>
#include <cstdint>
#include <mgpcl/Util.h>

enum FramebufferDepthMode
{
    kFDM_DepthStencilRenderbuffer,
    kFDM_DepthRenderbuffer,
    kFDM_DepthTexture
};

/*
 * Classe representant un Framebuffer OpenGL
 */
class Framebuffer
{
    M_NON_COPYABLE(Framebuffer)

public:
    Framebuffer();
    Framebuffer(uint32_t w, uint32_t h);
    Framebuffer(Framebuffer &&src);
    ~Framebuffer();

    /*
     * Initialise le framebuffer avec la largeur et la hauteur passe en
     * parametre via w et h, respectivement.
     *
     * C'est la premiere fonction qui doit etre appelee avant toute action
     * sur le framebuffer.
     */
    void init(uint32_t w, uint32_t h);

    /*
     * Cree un color buffer et l'ajoute au framebuffer. Le buffer aura pour
     * index et pour format les valeurs passees en parametres.
     */
    void createColorBuffer(int idx, gl::TextureFormat tf);

    /*
     * Cree un render buffer et l'ajoute au framebuffer. Il y a trois facons
     * d'executer cette operation, selectionnee via le parametre 'mode':
     *
     * - kFDM_DepthStencilRenderbuffer: Utiliser un renderbuffer de profondeur et de pochoir
     * - kFDM_DepthRenderbuffer       : Utiliser un renderbuffer de profondeur uniquement
     * - kFDM_DepthTexture            : Utiliser une texture pour faire office de renderbuffer de profondeur
     */
    void createDepthBuffer(FramebufferDepthMode mode = kFDM_DepthStencilRenderbuffer);

    /*
     * Finalise le framebuffer. Il faut au moins un color buffer pour finaliser un framebuffer.
     * Retourne true si le framebuffer a ete cree correctement. Si c'est le cas, le framebuffer
     * pourra ensuite etre utiliser comme cible de rendu.
     */
    bool finishFramebuffer();

    /*
     * Selectionne ce framebuffer comme cible de rendu. Le framebuffer doit auparavent
     * ete finalise avec 'finishFramebuffer()'.
     */
    void bindForRender()
    {
        gl::bindFramebuffer(gl::kFBT_Framebuffer, m_fbo);
        gl::viewport(0, 0, static_cast<GLsizei>(m_w), static_cast<GLsizei>(m_h));
    }

    /*
     * Selectionne la fenetre principale comme cible de rendu et restaure le
     * viewport associee, decrit par les parametres w et h (largeur et hauteur
     * de la fenetre OpenGL).
     */
    static void unbindFromRender(uint32_t w, uint32_t h)
    {
        gl::bindFramebuffer(gl::kFBT_Framebuffer, 0);
        gl::viewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
    }

    /*
     * Retourne l'ID de texture du color buffer ayant pour index 'idx'.
     */
    GLuint colorAttachmentID(int idx = 0) const
    {
        return m_color[idx];
    }

    /*
     * Retourne l'ID de renderbuffer/texture du buffer de profondeur.
     */
    GLuint depthAttachmentID() const
    {
        return m_depth;
    }

    Framebuffer &operator = (Framebuffer &&src);

    /*
     * Retourne la largeur, en pixels, de ce framebuffer.
     */
    uint32_t width() const
    {
        return m_w;
    }

    /*
     * Retourne la hauteur, en pixels, de ce framebuffer.
     */
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
    FramebufferDepthMode m_depthMode;
};
