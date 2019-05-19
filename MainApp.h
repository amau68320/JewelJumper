#pragma once
#include <GL/glew.h>
#include <mgpcl/ProgramArgs.h>
#include <mgpcl/Time.h>
#include <mgpcl/Matrix3.h>
#include <GLFW/glfw3.h>
#include <aiso/UIShader.h>
#include "GameObject.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Skybox.h"

enum JJShader
{
    kS_Main = 0,
    kS_FXAA,
    kS_Skybox,
    kS_Tonemap,
    kS_Wirefame,
    kS_BlurX,
    kS_BlurY,
    kS_NoOp,
    kS_Normal,

    //Garder en bas
    kS_Count
};

/*
 * Classe principale gerant JewelJumper.
 */
class MainApp
{
public:
	MainApp(GLFWwindow* wnd);
	~MainApp();

    /*
     * setup() initialise JewelJumper selon les arguments
     * de commandes (stockees dans pargs), et la taille de la
     * fenetre OpenGL (ww et wh).
     *
     * Retourne true si tout c'est bien passe, et false en cas
     * d'erreur.
     */
    bool setup(m::ProgramArgs &pargs, int ww, int wh);

    /*
     * run() lance la boucle principale du jeu. Cette fonction
     * ne retournera pas tant que l'utilisateur ne quitter pas
     * le jeu.
     */
	void run();

    /*
     * use3DShader() bind le shader (glUseProgram) et lui envoi les
     * matrices de modele, vue et projection.
     */
    void use3DShader(UIShader &shdr);

    /*
     * pushMatrix() sauvegarde la matrice de modele actuelle sur la pile,
     * et retourne une reference vers la nouvelle matrice a utiliser.
     */
    m::Matrix4f &pushMatrix()
    {
        m_model[m_curModelMat + 1] = m_model[m_curModelMat];
        return m_model[++m_curModelMat];
    }

    /*
     * popMatrix() ressort l'ancienne matrice de la pile et la retourne
     * en reference.
     */
    m::Matrix4f &popMatrix()
    {
        return m_model[--m_curModelMat];
    }

    /*
     * Retourne une reference vers le shader principal.
     */
    UIShader &mainShader()
    {
        if(m_override != nullptr)
            return *m_override;

        return m_useWireframe ? m_shaders[kS_Wirefame] : m_shaders[kS_Main];
    }

    /*
     * Retourne une reference vers la skybox
     */
    Skybox &skybox()
    {
        return m_skybox;
    }

    /*
     * Retourne une reference vers l'instance de MainApp.
     */
    static MainApp &instance()
    {
        return *m_instance;
    }

private:
    void loadShader(JJShader sdr, const char *name, bool hasGeom = false);
    void update(float dt);
    void render3D(float ptt);
    void handleMouseButtonEvent(int button, int action, int mods);
    void handleMouseMotionEvent(float dx, float dy);
    void handleKeyboardEvent(int key, int scancode, int action, int mods);
    void debugDrawTexture(GLuint tex);
    void grabMouse(bool grabbed);

	m::List<GameObject*> m_objects;
	double m_renderPeriod;
    double m_renderDelta;
	GLFWwindow* m_wnd;
    m::Matrix4f m_proj;
    m::Matrix4f m_view;
    m::Matrix4f m_model[16];
    int m_curModelMat;
    Camera *m_camera;
    double m_lastCursorPosX;
    double m_lastCursorPosY;

    UIShader m_shaders[kS_Count];
    UIShader *m_override;

    Framebuffer m_normalPass;
    Framebuffer m_hdrFBO0;
    //Framebuffer m_hdrFBO1;
    Framebuffer m_bloomFBO[2];
    Framebuffer m_sdrFBO;

    GLuint m_peVBO;
    GLuint m_peVAO;
    m::Vector2f m_invTexSize;
    m::Vector2f m_halfInvTexSize;
    bool m_fxaaEnable;
    Skybox m_skybox;
    bool m_useWireframe;
    uint32_t m_ww;
    uint32_t m_wh;
    m::Matrix3f m_2Dmat;
    bool m_doDebugDraw;
    GLuint m_numDrawcalls;
    bool m_relativeMouse;

    static MainApp *m_instance;
};
