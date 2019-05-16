#include "MainApp.h"
#include "Gem.h"
#include <mgpcl/Math.h>
#include <mgpcl/Logger.h>

#define UPDATE_INTERVAL 20.0

MainApp *MainApp::m_instance = nullptr;

MainApp::MainApp(GLFWwindow* wnd) : m_objects(), m_currentTimeUpdate(0.0), m_currentTimeRenderer(0.0),
                                    m_fps(0), m_curModelMat(0), m_lastCursorPosX(0.0), m_lastCursorPosY(0.0),
                                    m_peVBO(0), m_peVAO(0), m_fxaaEnable(true), m_useWireframe(false)
{
    m_instance = this;

	m_wnd = wnd;
    m_camera = new FreeCamera;
}

MainApp::~MainApp()
{
    if(m_peVAO != 0)
        gl::deleteVertexArray(m_peVAO);

    if(m_peVBO != 0)
        gl::deleteBuffer(m_peVBO);

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

    //Shaders
    if(!m_mainShader.load("shaders/main.vert", "shaders/main.frag")) {
        mlogger.error(M_LOG, "Impossible de charger le shader principal: %s", m_mainShader.errorString().raw());
        return false;
    }

    if(!m_fxaaShader.load("shaders/fxaa.vert", "shaders/fxaa.frag")) {
        mlogger.error(M_LOG, "Impossible de charger le shader principal: %s", m_fxaaShader.errorString().raw());
        return false;
    }

    if(!m_skyboxShader.load("shaders/skybox.vert", "shaders/skybox.frag")) {
        mlogger.error(M_LOG, "Impossible de charger le shader principal: %s", m_skyboxShader.errorString().raw());
        return false;
    }

    if(!m_tonemapShader.load("shaders/tonemap.vert", "shaders/tonemap.frag")) {
        mlogger.error(M_LOG, "Impossible de charger le shader principal: %s", m_tonemapShader.errorString().raw());
        return false;
    }

    if(!m_wireframeShader.load("shaders/wireframe.vert", "shaders/wireframe.geom", "shaders/wireframe.frag")) {
        mlogger.error(M_LOG, "Impossible de charger le shader fil de fer: %s", m_wireframeShader.errorString().raw());
        return false;
    }

    //Framebuffers
    m_mainFBO.init(static_cast<uint32_t>(ww), static_cast<uint32_t>(wh));
    m_mainFBO.createColorBuffer(gl::kTF_RGB16F);
    m_mainFBO.createDepthBuffer();

    if(!m_mainFBO.finishFramebuffer()) {
        mlogger.error(M_LOG, "Erreur lors de la creation du FBO principal");
        return false;
    }

    m_peFBO.init(static_cast<uint32_t>(ww), static_cast<uint32_t>(wh));
    m_peFBO.createColorBuffer(gl::kTF_RGB8);

    if(!m_peFBO.finishFramebuffer()) {
        mlogger.error(M_LOG, "Erreur lors de la creation du FBO de post-effet");
        return false;
    }

    //VBO & VAO de post-effets
    const float squareShape[] = {
        -1.0f, -1.0f,   1.0f, -1.0f,   1.0f, 1.0f,
        -1.0f, -1.0f,   -1.0f, 1.0f,   1.0f, 1.0f
    };

    m_peVBO = gl::genBuffer();
    gl::bindBuffer(gl::kBT_ArrayBuffer, m_peVBO);
    gl::bufferData(gl::kBT_ArrayBuffer, sizeof(squareShape), squareShape, gl::kBU_StaticDraw);
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);

    m_peVAO = gl::genVertexArray();
    gl::bindVertexArray(m_peVAO);
    gl::bindBuffer(gl::kBT_ArrayBuffer, m_peVBO);
    gl::enableVertexAttribArray(0);
    gl::vertexAttribPointer(0, 2, gl::kDT_Float, false, 0, nullptr);
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);
    gl::bindVertexArray(0);

    m_invTexSize.setX(1.0f / static_cast<float>(ww));
    m_invTexSize.setY(1.0f / static_cast<float>(wh));

    //Objets du jeu
    if(!m_skybox.load("textures/skybox.hdr")) {
        mlogger.error(M_LOG, "Impossible de charger la skybox HDR");
        return false;
    }

    Gem *gem = new Gem;
    gem->generate(6, 0.75f, 1.0f, 1.0f, 0.75f);

    m_objects.add(gem);
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
    /***************************** RENDU PRINCIPAL *****************************/
    //Effacer l'ecran et parametres de base
    m_mainFBO.bindForRender();
    gl::depthMask(true);
    gl::clear(gl::kCF_DepthBuffer);
    gl::disable(gl::kC_CullFace);

    //Matrice de vue
    m::Vector3f camPos;
    m_camera->getTransform(m_view, camPos, ptt);

    //RAZ de la pile de matrice modele
    m_curModelMat = 0;
    m_model[0].loadIdentity();

    //Rendu du ciel
    gl::disable(gl::kC_DepthTest);
    gl::depthMask(false);
    pushMatrix().translate(camPos);
    use3DShader(m_skyboxShader);
    m_skybox.draw();
    Shader::unbind();
    popMatrix();

    //Rendu des objets
    gl::depthMask(true);
    gl::enable(gl::kC_DepthTest);
    gl::enable(gl::kC_CullFace);

    for(GameObject *object : m_objects)
        object->render(ptt);

    Framebuffer::unbindFromRender();

    /***************************** RENDU DES EFFETS *****************************/
    gl::disable(gl::kC_DepthTest);
    gl::depthMask(false);
    gl::disable(gl::kC_Blend);
    gl::disable(gl::kC_CullFace);

    //Tone mapping
    m_peFBO.bindForRender();
    m_tonemapShader.bind();
    gl::bindTexture(gl::kTT_Texture2D, m_mainFBO.colorAttachmentID());
    gl::bindVertexArray(m_peVAO);
    gl::drawArrays(gl::kDM_Triangles, 0, 6);
    gl::bindVertexArray(0);
    gl::bindTexture(gl::kTT_Texture2D, 0);
    Shader::unbind();
    Framebuffer::unbindFromRender();

    //FXAA
    m_fxaaShader.bind();
    gl::uniform2f(m_fxaaShader.getUniformLocation("u_InvTexSize"), m_invTexSize.x(), m_invTexSize.y());
    gl::bindTexture(gl::kTT_Texture2D, m_peFBO.colorAttachmentID());
    gl::bindVertexArray(m_peVAO);
    gl::drawArrays(gl::kDM_Triangles, 0, 6);
    gl::bindVertexArray(0);
    gl::bindTexture(gl::kTT_Texture2D, 0);
    Shader::unbind();
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
        else if(key == GLFW_KEY_Q) {
            m_fxaaEnable = !m_fxaaEnable;
            mlogger.info(M_LOG, "FXAA: %s", m_fxaaEnable ? "ON" : "OFF");
        } else if(key == GLFW_KEY_Z) {
            m_useWireframe = !m_useWireframe;
            mlogger.info(M_LOG, "Wireframe: %s", m_useWireframe ? "ON" : "OFF");
        } else
            m_camera->onKeyDown(scancode);
    } else if(action == GLFW_RELEASE)
        m_camera->onKeyUp(scancode);
}
