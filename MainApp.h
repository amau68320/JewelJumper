#pragma once
#include <GL/glew.h>
#include <mgpcl/Time.h>
#include <mgpcl/Matrix3.h>
#include <mgpcl/SignalSlot.h>
#include <mgpcl/JSON.h>
#include <GLFW/glfw3.h>
#include <aiso/UIShader.h>
#include "GameObject.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Skybox.h"
#include "Histogram.h"

enum JJShader
{
    kS_Main = 0,
    kS_MainNoRT,
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

class UIElement;
class UIFontFace;
class UILabel;
class UIProgressBar;
class UIPushButton;

/*
 * Classe principale gerant JewelJumper.
 */
class MainApp : public m::SlotCapable
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
    bool setup(int ww, int wh);

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

        if(m_useWireframe)
            return m_shaders[kS_Wirefame];

        return m_shaders[m_internalRefraction ? kS_Main : kS_MainNoRT];
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
    bool changeColor(UIElement *e);
    bool onCheckboxValueChanged(UIElement *e);
    bool onSliderValueChanged(UIElement *e);
    void renderHUD();
    void renderLensFlare();
    bool onChangeSkyboxClicked(UIElement *e);

    //Contenu
	m::List<GameObject*> m_objects;
    Camera *m_camera;
    Skybox m_skybox;
    m::JSONElement m_skyboxData;
    m::Vector3f m_sunPosWorldSpace;

    //Timing
	double m_renderPeriod;
    double m_renderDelta;
    
    //Gestion de la fenetre
	GLFWwindow* m_wnd;
    bool m_relativeMouse;
    double m_lastCursorPosX;
    double m_lastCursorPosY;
    uint32_t m_ww;
    uint32_t m_wh;

    //Matrices
    m::Matrix4f m_proj;
    m::Matrix4f m_view;
    m::Matrix4f m_model[16];
    int m_curModelMat;
    m::Matrix3f m_2Dmat;

    //Shaders
    UIShader m_shaders[kS_Count];
    UIShader *m_override;

    //Framebuffers
    Framebuffer m_normalPass;
    Framebuffer m_hdrFBO0;
    Framebuffer m_bloomFBO[2];
    Framebuffer m_sdrFBO;

    //Uniforms
    m::Vector2f m_invTexSize;
    m::Vector2f m_halfInvTexSize;
    float m_exposure;
    float m_bloomThreshold;

    //Fonctionnalites
    bool m_fxaaEnable;
    bool m_useWireframe;
    bool m_doDebugDraw;
    bool m_internalRefraction;
    bool m_bloomEnable;
    bool m_displayDebugString;

    //Divers
    GLuint m_peVBO;
    GLuint m_peVAO;
    GLuint m_numDrawcalls;
    int m_oldSides;
    UIFontFace *m_font;
    m::String m_debugString;

    //Lens flare
    GLuint m_PBOs[2];
    int m_curPBO;
    m::Vector2i m_sunPos;
    float m_sunVisibility;
    GLuint m_lensFlareSprite;

    //Gestion des telechargements de skybox
    int m_lastDownload;
    UILabel *m_dlLabel;
    UIProgressBar *m_dlProgress;
    UIPushButton *m_skyboxBtn;
    int m_curSkybox;

    //Histogramme
    Histogram m_histo;

    static MainApp *m_instance;
};
