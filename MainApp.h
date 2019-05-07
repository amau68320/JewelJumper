#pragma once
#include <GL/glew.h>
#include <mgpcl/ProgramArgs.h>
#include <mgpcl/Time.h>
#include <GLFW/glfw3.h>
#include "GameObject.h"
#include "Camera.h"
#include "Shader.h"

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
    void use3DShader(Shader &shdr);

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

private:
    void render3D(float ptt);
    void handleMouseButtonEvent(int button, int action, int mods);
    void handleMouseMotionEvent(float dx, float dy);
    void handleKeyboardEvent(int key, int scancode, int action, int mods);

	m::List<GameObject*> m_objects;
	double m_currentTimeUpdate;
	double m_currentTimeRenderer;
	int m_fps;
	GLFWwindow* m_wnd;
    m::Matrix4f m_proj;
    m::Matrix4f m_view;
    m::Matrix4f m_model[16];
    int m_curModelMat;
    Camera *m_camera;
    double m_lastCursorPosX;
    double m_lastCursorPosY;
};
