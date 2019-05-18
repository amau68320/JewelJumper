#include "MainApp.h"
#include "Gem.h"
#include <mgpcl/Math.h>
#include <mgpcl/Logger.h>

#define UPDATE_INTERVAL 20.0

MainApp *MainApp::m_instance = nullptr;

MainApp::MainApp(GLFWwindow* wnd) : m_objects(), m_currentTimeUpdate(0.0), m_currentTimeRenderer(0.0),
                                    m_fps(0), m_curModelMat(0), m_lastCursorPosX(0.0), m_lastCursorPosY(0.0),
                                    m_override(nullptr), m_peVBO(0), m_peVAO(0), m_fxaaEnable(true),
                                    m_useWireframe(false), m_ww(0), m_wh(0)
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
    m_ww = static_cast<uint32_t>(ww);
    m_wh = static_cast<uint32_t>(wh);

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
    try {
        loadShader(kS_Main    , "main"           );
        loadShader(kS_FXAA    , "fxaa"           );
        loadShader(kS_Skybox  , "skybox"         );
        loadShader(kS_Tonemap , "tonemap"        );
        loadShader(kS_Wirefame, "wireframe", true);
        loadShader(kS_BlurX   , "blurX"          );
        loadShader(kS_BlurY   , "blurY"          );
        loadShader(kS_NoOp    , "noop"           );
        loadShader(kS_Normal  , "normal"         );
    } catch(const char *osef) {
        return false;
    }

    m_shaders[kS_Main].bind();
    gl::uniform1i(m_shaders[kS_Main].getUniformLocation("u_BackNormal"), 1);
    gl::uniform1i(m_shaders[kS_Main].getUniformLocation("u_BackDepth"), 2);
    Shader::unbind();

    m_shaders[kS_Tonemap].bind();
    gl::uniform1i(m_shaders[kS_Tonemap].getUniformLocation("u_BloomTex"), 1);
    Shader::unbind();

    //Framebuffers
    m_normalPass.init(static_cast<uint32_t>(ww), static_cast<uint32_t>(wh));
    m_normalPass.createColorBuffer(0, gl::kTF_RGB8);
    m_normalPass.createDepthBuffer(kFDM_DepthTexture);

    if(!m_normalPass.finishFramebuffer()) {
        mlogger.error(M_LOG, "Erreur lors de la finalisation du FBO de normales");
        return false;
    }

    m_hdrFBO0.init(static_cast<uint32_t>(ww), static_cast<uint32_t>(wh));
    m_hdrFBO0.createColorBuffer(0, gl::kTF_RGB16F);
    m_hdrFBO0.createColorBuffer(1, gl::kTF_RGB16F);
    m_hdrFBO0.createDepthBuffer();

    if(!m_hdrFBO0.finishFramebuffer()) {
        mlogger.error(M_LOG, "Erreur lors de la creation du FBO HDR 0");
        return false;
    }

    for(int i = 0; i < 2; i++) {
        m_bloomFBO[i].init(static_cast<uint32_t>(ww / 2), static_cast<uint32_t>(wh / 2));
        m_bloomFBO[i].createColorBuffer(0, gl::kTF_RGB16F);

        if(!m_bloomFBO[i].finishFramebuffer()) {
            mlogger.error(M_LOG, "Erreur lors de la creation du FBO de bloom %d", i);
            return false;
        }
    }

    m_sdrFBO.init(static_cast<uint32_t>(ww), static_cast<uint32_t>(wh));
    m_sdrFBO.createColorBuffer(0, gl::kTF_RGB8);

    if(!m_sdrFBO.finishFramebuffer()) {
        mlogger.error(M_LOG, "Erreur lors de la creation du FBO SDR");
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

    m_halfInvTexSize.setX(1.0f / static_cast<float>(ww / 2));
    m_halfInvTexSize.setY(1.0f / static_cast<float>(wh / 2));

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

void MainApp::loadShader(JJShader sdr, const char *name, bool hasGeom)
{
    m::String vert, geom, frag;
    vert += "shaders/";
    vert += name;
    vert += ".vert";

    frag += "shaders/";
    frag += name;
    frag += ".frag";

    if(hasGeom) {
        geom += "shaders/";
        geom += name;
        geom += ".geom";
    }

    if(!m_shaders[sdr].load(vert, geom, frag)) {
        mlogger.error(M_LOG, "Impossible de charger le shader \"%s\": %s", name, m_shaders[sdr].errorString().raw());
        throw name; //Parce-qu'il faut bien balancer quelque-chose...
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

static const GLuint g_allDrawBuffers[] = { gl::kFBA_ColorAttachment0, gl::kFBA_ColorAttachment1 };

void MainApp::render3D(float ptt)
{
    //Matrice de vue
    m::Vector3f camPos;
    m_camera->getTransform(m_view, camPos, ptt);

    //RAZ de la pile de matrice modele
    m_curModelMat = 0;
    m_model[0].loadIdentity();


    /***************************** RENDU NORMALES *****************************/
    m_normalPass.bindForRender();
    gl::depthMask(true);
    gl::clear(gl::kCF_ColorBuffer | gl::kCF_DepthBuffer);
    gl::enable(gl::kC_DepthTest);
    gl::enable(gl::kC_CullFace);
    gl::cullFace(gl::kF_Front);
    m_override = &m_shaders[kS_Normal];

    for(GameObject *object : m_objects)
        object->render(ptt);

    m_override = nullptr;
    

    /***************************** RENDU PRINCIPAL *****************************/
    //Effacer l'ecran et parametres de base
    m_hdrFBO0.bindForRender();
    gl::clear(gl::kCF_DepthBuffer);
    gl::disable(gl::kC_CullFace);
    gl::drawBuffers(2, g_allDrawBuffers);

    //Rendu du ciel
    gl::disable(gl::kC_DepthTest);
    gl::depthMask(false);
    pushMatrix().translate(camPos);
    use3DShader(m_shaders[kS_Skybox]);
    m_skybox.draw();
    Shader::unbind();
    popMatrix();

    //Rendu des objets
    gl::depthMask(true);
    gl::enable(gl::kC_DepthTest);
    gl::enable(gl::kC_CullFace);
    gl::cullFace(gl::kF_Back);

    use3DShader(m_shaders[kS_Main]);
    gl::uniform3f(m_shaders[kS_Main].getUniformLocation("u_CamPos"), camPos.x(), camPos.y(), camPos.z());
    Shader::unbind();

    gl::activeTexture(1);
    gl::bindTexture(gl::kTT_Texture2D, m_normalPass.colorAttachmentID());
    gl::activeTexture(2);
    gl::bindTexture(gl::kTT_Texture2D, m_normalPass.depthAttachmentID());
    gl::activeTexture(0);

    for(GameObject *object : m_objects)
        object->render(ptt);

    gl::activeTexture(1);
    gl::bindTexture(gl::kTT_Texture2D, 0);
    gl::activeTexture(2);
    gl::bindTexture(gl::kTT_Texture2D, 0);
    gl::activeTexture(0);
    gl::drawBuffer(gl::kFBA_ColorAttachment0);

    /***************************** RENDU DES EFFETS *****************************/
    gl::disable(gl::kC_DepthTest);
    gl::depthMask(false);
    gl::disable(gl::kC_Blend);
    gl::disable(gl::kC_CullFace);

    //BLOOM: Reduction de la taille
    m_bloomFBO[0].bindForRender();
    m_shaders[kS_NoOp].bind();
    gl::bindTexture(gl::kTT_Texture2D, m_hdrFBO0.colorAttachmentID(1));
    gl::bindVertexArray(m_peVAO);
    gl::drawArrays(gl::kDM_Triangles, 0, 6);
    gl::bindVertexArray(0);
    gl::bindTexture(gl::kTT_Texture2D, 0);
    Shader::unbind();

    //BLOOM: Flou
    for(int i = 0; i < 2; i++) {
        Shader &shdr = m_shaders[(i == 0) ? kS_BlurX : kS_BlurY];

        m_bloomFBO[1 - i].bindForRender();
        shdr.bind();
        gl::uniform2f(shdr.getUniformLocation("u_InvTexSize"), m_halfInvTexSize.x(), m_halfInvTexSize.y());
        gl::bindTexture(gl::kTT_Texture2D, m_bloomFBO[i].colorAttachmentID());
        gl::bindVertexArray(m_peVAO);
        gl::drawArrays(gl::kDM_Triangles, 0, 6);
        gl::bindVertexArray(0);
        gl::bindTexture(gl::kTT_Texture2D, 0);
        Shader::unbind();
    }

    //Tone mapping + bloom Y
    m_sdrFBO.bindForRender();
    m_shaders[kS_Tonemap].bind();
    gl::bindTexture(gl::kTT_Texture2D, m_hdrFBO0.colorAttachmentID());
    gl::activeTexture(1);
    gl::bindTexture(gl::kTT_Texture2D, m_bloomFBO[0].colorAttachmentID());
    gl::bindVertexArray(m_peVAO);
    gl::drawArrays(gl::kDM_Triangles, 0, 6);
    gl::bindVertexArray(0);
    gl::bindTexture(gl::kTT_Texture2D, 0);
    gl::activeTexture(0);
    gl::bindTexture(gl::kTT_Texture2D, 0);
    Shader::unbind();
    Framebuffer::unbindFromRender(m_ww, m_wh);

    //FXAA
    m_shaders[kS_FXAA].bind();
    gl::uniform2f(m_shaders[kS_FXAA].getUniformLocation("u_InvTexSize"), m_invTexSize.x(), m_invTexSize.y());
    gl::bindTexture(gl::kTT_Texture2D, m_sdrFBO.colorAttachmentID());
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
        } else if(key == GLFW_KEY_C) {
            Camera *oldCam = m_camera;

            if(dynamic_cast<RotatingCamera *>(m_camera) == nullptr) {
                m_camera = new RotatingCamera;
                glfwSetInputMode(m_wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                m_camera = new FreeCamera;
                glfwSetInputMode(m_wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }

            delete oldCam;
        } else
            m_camera->onKeyDown(scancode);
    } else if(action == GLFW_RELEASE)
        m_camera->onKeyUp(scancode);
}
