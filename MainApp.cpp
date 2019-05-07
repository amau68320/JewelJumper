#include "MainApp.h"
#include "Cube.h"
#include <mgpcl/Math.h>
#include <mgpcl/Logger.h>

#define UPDATE_INTERVAL 20.0

MainApp *MainApp::m_instance = nullptr;

MainApp::MainApp(GLFWwindow* wnd) : m_objects(), m_currentTimeUpdate(0.0), m_currentTimeRenderer(0.0),
                                    m_fps(0), m_curModelMat(0), m_lastCursorPosX(0.0), m_lastCursorPosY(0.0)
{
    m_instance = this;

	m_wnd = wnd;
    m_camera = new FreeCamera;
}

MainApp::~MainApp()
{
    for(GameObject *gob : m_objects)
        delete gob;

    delete m_camera;
}

bool MainApp::setup(m::ProgramArgs &pargs, int ww, int wh)
{
    //Handlers d'events
    glfwSetMouseButtonCallback(m_wnd, [] (GLFWwindow *, int button, int action, int mods) {
        m_instance->handleMouseButtonEvent(button, action, mods);
    });

    glfwSetCursorPosCallback(m_wnd, [] (GLFWwindow *, double x, double y) {
        double dx = x - m_instance->m_lastCursorPosX;
        double dy = y - m_instance->m_lastCursorPosY;
        m_instance->m_lastCursorPosX = x;
        m_instance->m_lastCursorPosY = y;

        m_instance->handleMouseMotionEvent(static_cast<float>(dx), static_cast<float>(dy));
    });

    glfwSetKeyCallback(m_wnd, [] (GLFWwindow *, int key, int scancode, int action, int mods) {
        m_instance->handleKeyboardEvent(key, scancode, action, mods);
    });

    //Creation de la matrice de projection, qui ne devrait pas changer avec le temps...
    m_proj = m::Matrix4f::perspective(80.0f * static_cast<float>(M_PI) / 180.0f,
                                      static_cast<float>(ww) / static_cast<float>(wh),
                                      0.01f, 1000.0f);
    //Capture du curseur
    m_lastCursorPosX = static_cast<double>(ww / 2);
    m_lastCursorPosY = static_cast<double>(wh / 2);
    glfwSetCursorPos(m_wnd, m_lastCursorPosX, m_lastCursorPosY);
    glfwSetInputMode(m_wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    m_camera->activate();

    //Shader
    if(!m_mainShader.load("shaders/main.vert", "shaders/main.frag")) {
        mlogger.error(M_LOG, "Impossible de charger le shader principal: %s", m_mainShader.errorString().raw());
        return false;
    }

    m_objects.add(new Cube);
    return true;
}

void MainApp::run()
{
	double waitTime = 0.0;
	double tmpUpdate = 0.0;
	m_currentTimeUpdate = m::time::getTimeMs();
	m_currentTimeRenderer = m::time::getTimeMs();

	while (glfwWindowShouldClose(m_wnd) == GLFW_FALSE)
	{
		glfwPollEvents();

		if ((m::time::getTimeMs() - m_currentTimeUpdate) >= 1000.0 / UPDATE_INTERVAL)
		{
			tmpUpdate = m_currentTimeUpdate;
			m_currentTimeUpdate = m::time::getTimeMs();
            update(static_cast<float>((m::time::getTimeMs() - tmpUpdate) / 1000.0));
		}

		m_currentTimeRenderer = m::time::getTimeMs();
        render3D(static_cast<float>((m::time::getTimeMs() - m_currentTimeUpdate) / (1000.0 / UPDATE_INTERVAL)));
		glfwSwapBuffers(m_wnd);

        if(m_fps > 0) {
            waitTime = m::math::minimum((1000.0 / UPDATE_INTERVAL) - (m::time::getTimeMs() - m_currentTimeUpdate),
                (1000.0 / m_fps) - (m::time::getTimeMs() - m_currentTimeRenderer));

            if(waitTime >= 10)
                m::time::sleepMs(static_cast<uint32_t>(waitTime));
        }
	}
}

void MainApp::update(float dt)
{
    m_camera->update(dt);

    for(GameObject *object : m_objects)
    {
        object->update(dt);
    }
}

void MainApp::render3D(float ptt)
{
    //Effacer l'ecran et parametres de base
    gl::depthMask(true);
    gl::clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl::clear(gl::kCF_ColorBuffer | gl::kCF_DepthBuffer);
    gl::enable(gl::kC_DepthTest);
    gl::disable(gl::kC_CullFace);

    //Matrice de vue
    m::Vector3f camPos; //OSEF pour l'instant
    m_camera->getTransform(m_view, camPos, ptt);

    //RAZ de la pile de matrice modele
    m_curModelMat = 0;
    m_model[0].loadIdentity();

    //Rendu des objets
    for(GameObject *object : m_objects)
    {
        object->render(ptt);
    }
}

void MainApp::use3DShader(Shader &shdr)
{
    shdr.bind();
    gl::uniformMatrix4fv(shdr.getUniformLocation("u_Projection"), 1, false, m_proj.data());
    gl::uniformMatrix4fv(shdr.getUniformLocation("u_View"), 1, false, m_view.data());
    gl::uniformMatrix4fv(shdr.getUniformLocation("u_Model"), 1, false, m_model[m_curModelMat].data());
}

void MainApp::handleMouseButtonEvent(int button, int action, int mods)
{
}

void MainApp::handleMouseMotionEvent(float dx, float dy)
{
    m_camera->onMouseMove(dx, dy);
}

void MainApp::handleKeyboardEvent(int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS) {
        if(key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(m_wnd, GLFW_TRUE);
        else
            m_camera->onKeyDown(scancode);
    } else if(action == GLFW_RELEASE)
        m_camera->onKeyUp(scancode);
}
