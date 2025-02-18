#include "MainApp.h"
#include "Gem.h"
#include "DownloadManager.h"
#include <mgpcl/Math.h>
#include <mgpcl/Logger.h>
#include <mgpcl/FileIOStream.h>
#include <mgpcl/File.h>
#include <aiso/UICore.h>
#include <aiso/UILoader.h>
#include <aiso/UISlider.h>
#include <aiso/UICheckBox.h>
#include <aiso/UILabel.h>
#include <aiso/UIProgressBar.h>
#include <aiso/UIPushButton.h>
#include <aiso/UIFontLibrary.h>
#include <aiso/UIImage.h>

#define JJ_UPDATE_PERIOD (1000.0 / 20.0)
#define JJ_SLEEP_THRESHOLD 10.0
#define JJ_LENS_FLARE_MEASURE 32

#ifdef _DEBUG
#define JJ_DEBUGSTR_DEFAULT true
#else
#define JJ_DEBUGSTR_DEFAULT false
#endif

MainApp *MainApp::m_instance = nullptr;

MainApp::MainApp(GLFWwindow* wnd) : m_wnd(wnd), m_relativeMouse(false), m_lastCursorPosX(0.0), m_lastCursorPosY(0.0),
                                    m_ww(0), m_wh(0), m_curModelMat(0), m_override(nullptr), m_exposure(2.5f),
                                    m_bloomThreshold(4.75f), m_fxaaEnable(true), m_useWireframe(false), m_doDebugDraw(false),
                                    m_internalRefraction(true), m_bloomEnable(true), m_displayDebugString(JJ_DEBUGSTR_DEFAULT),
                                    m_lensFlareEnable(true), m_peVBO(0), m_peVAO(0), m_numDrawcalls(0), m_oldSides(6),
                                    m_font(nullptr), m_PBOs{ 0, 0 }, m_curPBO(0), m_sunVisibility(0.0f), m_lensFlareSprite(0),
                                    m_lastDownload(-1), m_dlLabel(nullptr), m_dlProgress(nullptr), m_skyboxBtn(nullptr),
                                    m_curSkybox(0), m_bloomThresholdSlider(nullptr), m_frameTimeGraph(32), m_frameNumGraph(32),
                                    m_ftMins{ 0, 0, 0, 0 }, m_ftMaxs{ 0, 0, 0, 0 }, m_displayStats(false), m_viewWindow(nullptr),
                                    m_customizeWindow(nullptr)
{
    m_instance = this;

    m_renderPeriod = 0.0; //IPS illimite par defaut
    m_renderDelta = 1000.0 / 60.0;

    m_histo = new Histogram;
    m_freeCam = new FreeCamera;
    m_orbitalCam = new RotatingCamera;
    m_camera = m_orbitalCam;

    DownloadManager::create();
}

MainApp::~MainApp()
{
    //Interface utilisateur
    if(m_dlLabel != nullptr)
        m_dlLabel->removeRef();

    if(m_dlProgress != nullptr)
        m_dlProgress->removeRef();

    if(m_skyboxBtn != nullptr)
        m_skyboxBtn->removeRef();

    if(m_bloomThresholdSlider != nullptr)
        m_bloomThresholdSlider->removeRef();

    if(m_viewWindow != nullptr)
        m_viewWindow->removeRef();

    if(m_customizeWindow != nullptr)
        m_customizeWindow->removeRef();

    uiCore.destroy();
    delete m_histo;

    //VBO+VAO post-effets
    if(m_peVAO != 0)
        gl::deleteVertexArray(m_peVAO);

    if(m_peVBO != 0)
        gl::deleteBuffer(m_peVBO);

    for(int i = 0; i < 2; i++) {
        if(m_PBOs[i] != 0)
            gl::deleteBuffer(m_PBOs[i]);
    }

    //Objets du jeu
    for(GameObject *gob : m_objects)
        delete gob;

    //Cameras
    delete m_freeCam;
    delete m_orbitalCam;

    //Gestionnaire de telechargement
    dlMgr.onQuit();
    DownloadManager::destroy();
}

void MainApp::cleanup()
{
    if(m_displayStats) {
        mlogger.info(M_LOG, "=========== PERFS FINALES ===========");
        for(int i = 0; i < 4; i++)
            mlogger.info(M_LOG, "meilleure[%d] = %d", i, m_ftMins[i]);

        mlogger.info(M_LOG, "");
        for(int i = 0; i < 4; i++)
            mlogger.info(M_LOG, "pire[%d] = %d", i, m_ftMaxs[i]);

        mlogger.info(M_LOG, "");
    }
}

bool MainApp::setup(int ww, int wh, GLuint histoDiv0)
{
    m_ww = static_cast<uint32_t>(ww);
    m_wh = static_cast<uint32_t>(wh);

    //Handlers d'events
    glfwSetMouseButtonCallback(m_wnd, [] (GLFWwindow *, int button, int action, int mods) {
        m_instance->handleMouseButtonEvent(button, action, mods);
    });

    glfwSetCursorPosCallback(m_wnd, [] (GLFWwindow *, double x, double y) {
        if(m_instance->m_relativeMouse) {
            double dx = x - m_instance->m_lastCursorPosX;
            double dy = y - m_instance->m_lastCursorPosY;
            m_instance->m_lastCursorPosX = x;
            m_instance->m_lastCursorPosY = y;

            m_instance->handleMouseMotionEvent(static_cast<float>(dx), static_cast<float>(dy));
        } else
            uiCore.handleMouseMotionEvent(static_cast<int>(x), static_cast<int>(y));
    });

    glfwSetKeyCallback(m_wnd, [] (GLFWwindow *, int key, int scancode, int action, int mods) {
        m_instance->handleKeyboardEvent(key, scancode, action, mods);
    });

    glfwSetScrollCallback(m_wnd, [] (GLFWwindow *, double ax, double ay) {
        m_instance->handleMouseScrollEvent(static_cast<float>(ax), static_cast<float>(ay));
    });

    //Creation de la matrice de projection, qui ne devrait pas changer avec le temps...
    m_proj = m::Matrix4f::perspective(80.0f * static_cast<float>(M_PI) / 180.0f,
                                      static_cast<float>(ww) / static_cast<float>(wh),
                                      0.1f, 10.0f);

    //Shaders
    try {
        m_shaders[kS_MainNoRT].addDefine("NO_RAYTRACE");

        loadShader(kS_Main    , "main"           );
        loadShader(kS_MainNoRT, "main"           );
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

    const float rtStepX = 2.0f / static_cast<float>(ww);
    const float rtStepY = 2.0f / static_cast<float>(wh);

    m_shaders[kS_Main].bind();
    gl::uniform1i(m_shaders[kS_Main].getUniformLocation("u_BackNormal"), 1);
    gl::uniform1i(m_shaders[kS_Main].getUniformLocation("u_BackDepth"), 2);
    gl::uniform1f(m_shaders[kS_Main].getUniformLocation("u_RaytraceStep"), m::math::minimum(rtStepX, rtStepY));
    UIShader::unbind();

    m_shaders[kS_Tonemap].bind();
    gl::uniform1i(m_shaders[kS_Tonemap].getUniformLocation("u_BloomTex"), 1);
    UIShader::unbind();

    //Framebuffers
    m_normalPass.init(static_cast<uint32_t>(ww), static_cast<uint32_t>(wh));
    m_normalPass.createColorBuffer(0, gl::kTF_RGB8);
    m_normalPass.createDepthBuffer(kFDM_DepthTexture);

    if(!m_normalPass.finishFramebuffer()) {
        mlogger.error(M_LOG, "Erreur lors de la finalisation du FBO de normales");
        return false;
    }

    m_hdrFBO0.init(static_cast<uint32_t>(ww), static_cast<uint32_t>(wh));
    m_hdrFBO0.createColorBuffer(0, gl::kTF_RGBA16F);
    m_hdrFBO0.createColorBuffer(1, gl::kTF_RGBA16F);
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

    //Lens flare
    for(int i = 0; i < 2; i++) {
        m_PBOs[i] = gl::genBuffer();

        gl::bindBuffer(gl::kBT_PixelPackBuffer, m_PBOs[i]);
        gl::bufferData(gl::kBT_PixelPackBuffer, JJ_LENS_FLARE_MEASURE * JJ_LENS_FLARE_MEASURE * sizeof(float), nullptr, gl::kBU_StreamRead);
        gl::bindBuffer(gl::kBT_PixelPackBuffer, 0);
    }

    UIImage *lensFlareImg = UIImage::loadFrom("textures/lens_flare.png", kUIILF_AddAlpha);
    if(lensFlareImg == nullptr) {
        mlogger.error(M_LOG, "Impossible de charger le sprite de lens flare");
        return false;
    }

    m_lensFlareSprite = lensFlareImg->makeGLTexture(0, gl::kTF_Linear, gl::kTF_Linear);
    delete lensFlareImg;

    //Histogramme
    if(!m_histo->setup(m_ww, m_wh, histoDiv0))
        return false;

    //Gestion des skyboxes
    m::SSharedPtr<m::FileInputStream> fis(new m::FileInputStream);

    if(fis->open("textures/skyboxes.json") != m::FileInputStream::kOE_Success) {
        mlogger.error(M_LOG, "Impossible d'ouvrir la liste des skybox");
        return false;
    }

    m::String skyboxParseError;
    if(!m::json::parse(fis.staticCast<m::InputStream>(), m_skyboxData, skyboxParseError)) {
        mlogger.error(M_LOG, "Impossible de lire la liste des skybox: %s", skyboxParseError.raw());
        return false;
    }
    
    fis->close();

    if(!m_skyboxData.isArray() || m_skyboxData.size() <= 0 || !m_skyboxData[0].isObject()) {
        mlogger.error(M_LOG, "La liste des skybox contient des donnees invalides.");
        return false;
    }

    const m::JSONElement &sunPos = m_skyboxData[0]["sunPos"];
    if(sunPos.isArray() && sunPos.size() == 3)
        m_sunPosWorldSpace = m::Vector3d(sunPos[0].asDouble(), sunPos[1].asDouble(), sunPos[2].asDouble()).cast<float>();

    if(!m_skybox.load("textures/skybox.hdr")) {
        mlogger.error(M_LOG, "Impossible de charger la skybox HDR");
        return false;
    }

    //Telechargement de skybox additionnelles
    m::File textureRoot("textures");
    bool shouldDlSkyboxes = false;

    for(int i = 1; i < m_skyboxData.size(); i++) {
        const m::JSONElement &fname = m_skyboxData[i]["filename"];

        if(fname.isString()) {
            m::File fle(textureRoot, fname.asString());

            if(!fle.exists()) {
                shouldDlSkyboxes = true;
                dlMgr.queueDownload(fname.asString());
            }
        }
    }

    //Objets du jeu
    Gem *gem = new Gem;
    gem->generate(m_oldSides, 0.75f, 1.0f, 1.0f, 0.75f);
    m_objects.add(gem);

    //Interface utilisateur
    UICore::create().setup(ww, wh);

    try {
        UIWindow *wnd = new UIWindow;
        ui::load("ruby", wnd);

        wnd->byName<UISlider>("sCR")->setValue(1.0f)->onValueChanged.connect(this, &MainApp::changeColor);
        wnd->byName<UISlider>("sCG")->setValue(0.0f)->onValueChanged.connect(this, &MainApp::changeColor);
        wnd->byName<UISlider>("sCB")->setValue(0.0f)->onValueChanged.connect(this, &MainApp::changeColor);

        wnd->byName<UISlider>("sIOR")->setRange(1.0f, 2.0f)->setValue(1.45f)->onValueChanged.connect(this, &MainApp::onSliderValueChanged);

        wnd->byName<UISlider>("sN")->setRange(3.0f, 10.0f, true)->setValue(6)->onValueChanged.connect(this, &MainApp::changeShape);
        wnd->byName<UISlider>("sH" )->setRange(0.1f, 2.0f)->setValue(0.75f)->onValueChanged.connect(this, &MainApp::changeShape);
        wnd->byName<UISlider>("sR" )->setRange(0.1f, 2.0f)->setValue(1.0f )->onValueChanged.connect(this, &MainApp::changeShape);
        wnd->byName<UISlider>("sDH")->setRange(0.0f, 0.5f)->setValue(0.25f)->onValueChanged.connect(this, &MainApp::changeShape);
        wnd->byName<UISlider>("sDR")->setRange(0.0f, 1.0f)->setValue(0.25f)->onValueChanged.connect(this, &MainApp::changeShape);

        wnd->pack(false, true);
        wnd->setPos(ww - wnd->rect().width() - 10, wh - wnd->rect().height() - 10);
        uiCore.addWindow(wnd);
        m_customizeWindow = wnd;
    } catch(UILoadException &ex) {
        mlogger.error(M_LOG, "Erreur lors de la lecture du fichier d'interface graphique (ruby): %s", ex.what());
        return false;
    } catch(UIUnknownElementException &ex) {
        mlogger.error(M_LOG, "Il manque un element d'interface graphique (ruby): %s", ex.what());
        return false;
    }

    try {
        UIWindow *wnd = new UIWindow;
        ui::load("view", wnd);

        wnd->byName<UISlider>("sFOV")->setRange(60.0f, 150.0f)->setValue(80.0f)->onValueChanged.connect(this, &MainApp::onSliderValueChanged);
        wnd->byName<UISlider>("sTemporalAdaptation")->setRange(-2.3f, 0.7f)->setValue(std::log(m_histo->temporalAdaptationFactor()))->onValueChanged.connect(this, &MainApp::onSliderValueChanged);
        wnd->byName<UISlider>("sCamSpeed")->setRange(0.1f, 2.0f)->setValue(0.5f)->onValueChanged.connect(this, &MainApp::onSliderValueChanged);

        m_bloomThresholdSlider = wnd->byName<UISlider>("sBloomThreshold");
        m_bloomThresholdSlider->addRef();
        m_bloomThresholdSlider->setMax(32.0f)->setValue(m_bloomThreshold)->onValueChanged.connect(this, &MainApp::onSliderValueChanged);

        wnd->byName<UICheckBox>("cbRefraction")->setChecked()->onChanged.connect(this, &MainApp::onCheckboxValueChanged);
        wnd->byName<UICheckBox>("cbBloom"     )->setChecked()->onChanged.connect(this, &MainApp::onCheckboxValueChanged);
        wnd->byName<UICheckBox>("cbFXAA"      )->setChecked()->onChanged.connect(this, &MainApp::onCheckboxValueChanged);
        wnd->byName<UICheckBox>("cbVSync"     )->setChecked()->onChanged.connect(this, &MainApp::onCheckboxValueChanged);
        wnd->byName<UICheckBox>("cbLensFlare" )->setChecked()->onChanged.connect(this, &MainApp::onCheckboxValueChanged);
        wnd->byName<UICheckBox>("cbInfos"     )->setChecked(JJ_DEBUGSTR_DEFAULT)->onChanged.connect(this, &MainApp::onCheckboxValueChanged);
        wnd->byName<UICheckBox>("cbLimitFPS"  )->onChanged.connect(this, &MainApp::onCheckboxValueChanged);
        wnd->byName<UICheckBox>("cbWireframe" )->onChanged.connect(this, &MainApp::onCheckboxValueChanged);
        wnd->byName<UICheckBox>("cbDebugDraw" )->onChanged.connect(this, &MainApp::onCheckboxValueChanged);

        m_skyboxBtn = wnd->byName<UIPushButton>("btnSkybox");
        m_skyboxBtn->onClicked.connect(this, &MainApp::onChangeSkyboxClicked);
        m_skyboxBtn->addRef();

        wnd->pack(false, true);
        wnd->setPos(10, wh - wnd->rect().height() - 10);
        uiCore.addWindow(wnd);
        m_viewWindow = wnd;
    } catch(UILoadException &ex) {
        mlogger.error(M_LOG, "Erreur lors de la lecture du fichier d'interface graphique (view): %s", ex.what());
        return false;
    } catch(UIUnknownElementException &ex) {
        mlogger.error(M_LOG, "Il manque un element d'interface graphique (view): %s", ex.what());
        return false;
    }

    if(shouldDlSkyboxes) {
        try {
            UIWindow *wnd = new UIWindow;
            ui::load("download", wnd);

            m_dlLabel = wnd->byName<UILabel>("lLabel");
            m_dlProgress = wnd->byName<UIProgressBar>("pbProgress");
            m_dlLabel->addRef();
            m_dlProgress->addRef();

            wnd->pack(false, true);
            wnd->setPos(ww - wnd->rect().width() - 10, 10);
            uiCore.addWindow(wnd);
            wnd->removeRef();
        } catch(UILoadException &ex) {
            mlogger.error(M_LOG, "Erreur lors de la lecture du fichier d'interface graphique (download): %s", ex.what());
            return false;
        } catch(UIUnknownElementException &ex) {
            mlogger.error(M_LOG, "Il manque un element d'interface graphique (download): %s", ex.what());
            return false;
        }

        dlMgr.startDownloadManager();
    }

    m_font = uiFontLib.get("Roboto-Regular", 12);
    glfwSetInputMode(m_wnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    return true;
}

void MainApp::run()
{
    double nextUpdate = m::time::getTimeMs() + JJ_UPDATE_PERIOD;
    double nextRender = m::time::getTimeMs() + m_renderPeriod;

    while(glfwWindowShouldClose(m_wnd) == GLFW_FALSE) {
        //Lecture des evenements
        glfwPollEvents();

        //MAJ
        double t = m::time::getTimeMs();
        if(t >= nextUpdate) {
            t += JJ_UPDATE_PERIOD;
            float dt = static_cast<float>((t - nextUpdate) / 1000.0);
            nextUpdate = t;
            update(dt);
            t = m::time::getTimeMs();
        }

        //Rendu
        if(t >= nextRender) {
            m_renderDelta = t - nextRender + m_renderPeriod;
            nextRender = t + m_renderPeriod; //FIXME: nextRender = nextRender + m_renderPeriod?

            float ptt = static_cast<float>((nextUpdate - t) / JJ_UPDATE_PERIOD);
            if(ptt <= 0.0f)
                ptt = 1.0f;
            else if(ptt >= 1.0f)
                ptt = 0.0f;
            else
                ptt = 1.0f - ptt;

            const int frameNum = m_histo->dispatchPos();
            render3D(ptt);
            gl::depthMask(true);
            gl::clear(gl::kCF_DepthBuffer);
            uiCore.render(static_cast<float>(t));
            renderHUD();

            m_numDrawcalls = gl::numDrawcalls;
            gl::numDrawcalls = 0;
            glfwSwapBuffers(m_wnd);

            double prevT = t;
            t = m::time::getTimeMs();

            m_frameTimeGraph.push(static_cast<int>((t - prevT) * 1000.0));
            m_frameNumGraph.push(frameNum);
        }

        //Mettre en pause le programme, eventuellement...
        double pause = m::math::minimum(nextUpdate, nextRender) - t;
        if(pause >= JJ_SLEEP_THRESHOLD)
            m::time::sleepMs(static_cast<uint32_t>(pause) - 5); //-5ms pour avoir de la marge
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

static double g_lastTime = 0.0;

void MainApp::update(float dt)
{
    //MAJ de la camera et de l'IHM
    m_camera->update(dt);
    uiCore.update(dt);

    //MAJ des objets du jeu
    for(GameObject *object : m_objects)
    {
        object->update(dt);
    }

    //MAJ des infos de debug
    if(m_displayDebugString) {
        m_debugString.cleanup();
        m_debugString.append("IPS: ", 5);
        m_debugString += m::String::fromDouble(1000.0 / m_renderDelta, 2);
        m_debugString.append("\nDrawcalls: ", 12);
        m_debugString += m::String::fromUInteger(m_numDrawcalls);
        m_debugString.append("\nExposition: ", 13);
        m_debugString += m::String::fromDouble(static_cast<double>(m_exposure), 4);
    }

    //MAJ de l'avancement des telechargements des skyboxes
    if(m_dlLabel != nullptr && m_dlProgress != nullptr) {
        int curFile = dlMgr.currentFile();
        float progress = dlMgr.progress();
        bool updateProgress = true;

        if(curFile != m_lastDownload) {
            m_lastDownload = curFile;

            if(curFile < 0) {
                UIWindow *wnd = static_cast<UIWindow *>(m_dlLabel->parent()); //FIXME: c'est pas bien ca
                m_dlLabel->removeRef();
                m_dlProgress->removeRef();
                uiCore.deleteNextTick(wnd);

                m_dlLabel = nullptr;
                m_dlProgress = nullptr;
                updateProgress = false;
            } else
                m_dlLabel->setText(dlMgr.fileName(curFile));
        }

        if(updateProgress && m_dlProgress->progress() != progress)
            m_dlProgress->setProgress(progress);
    }

    //MAJ du chargemenet asynchrone de la skybox
    if(m_skybox.update()) {
        //On entre ici que si une nouvelle skybox vient de finir d'etre chargee
        m::JSONElement &jsonData = m_skyboxData[m_curSkybox];

        if(jsonData.has("sunPos") && jsonData["sunPos"].isArray() && jsonData["sunPos"].size() == 3) {
            m_sunPosWorldSpace = m::Vector3d(jsonData["sunPos"][0].asDouble(), jsonData["sunPos"][1].asDouble(), jsonData["sunPos"][2].asDouble()).cast<float>();
            m_sunPosWorldSpace *= 10.0f;
        } else
            m_sunPosWorldSpace.set(0.0f);

        if(jsonData.has("bloomThreshold") && jsonData["bloomThreshold"].isNumber()) {
            m_bloomThreshold = static_cast<float>(jsonData["bloomThreshold"].asDouble());
            
            if(m_bloomThresholdSlider != nullptr)
                m_bloomThresholdSlider->setValue(m_bloomThreshold);
        }

        if(m_skyboxBtn != nullptr)
            m_skyboxBtn->setDisabled(false);
    }

    //Calcul de statistiques pour le profilage
    if(m_displayStats) {
        double ct = m::time::getTimeMs();
        if(ct - g_lastTime >= 250.0) {
            g_lastTime = ct;

            double sums[4] = { 0.0, 0.0, 0.0, 0.0 };
            int count[4] = { 0, 0, 0, 0 };

            for(int i = 0; i < ~m_frameTimeGraph; i++) {
                const int t = m_frameTimeGraph[i];
                const int n = m_frameNumGraph[i];

                sums[n] += static_cast<double>(t);
                count[n]++;
            }

            int maxPerf = 0;
            int maxPerfId = -1;
            int minPerf = 0x7FFFFFFF;
            int minPerfId = -1;

            mlogger.info(M_LOG, "=========== PERFS ===========");
            for(int i = 0; i < 4; i++) {
                const int perf = static_cast<int>(sums[i] / static_cast<double>(count[i]));
                mlogger.info(M_LOG, "perfs[%d] = %d us", i, perf);

                if(perf > maxPerf) {
                    maxPerf = perf;
                    maxPerfId = i;
                }

                if(perf < minPerf) {
                    minPerf = perf;
                    minPerfId = i;
                }
            }

            mlogger.info(M_LOG, "Pire perf trame %d", maxPerfId);
            mlogger.info(M_LOG, "Meilleure perf trame %d", minPerfId);
            mlogger.info(M_LOG, "");
            m_ftMaxs[maxPerfId]++;
            m_ftMins[minPerfId]++;
        }
    }
}

static const GLuint g_allDrawBuffers[] = { gl::kFBA_ColorAttachment0, gl::kFBA_ColorAttachment1 };

void MainApp::render3D(float ptt)
{
    m_exposure = m_histo->computedExposure();

    //Matrice de vue
    m::Vector3f camPos;
    m_camera->getTransform(m_view, camPos, ptt);

    //RAZ de la pile de matrice modele
    m_curModelMat = 0;
    m_model[0].loadIdentity();
    gl::depthMask(true);


    if(m_internalRefraction) {
        /***************************** RENDU NORMALES *****************************/
        m_normalPass.bindForRender();
        gl::clear(gl::kCF_ColorBuffer | gl::kCF_DepthBuffer);
        gl::enable(gl::kC_DepthTest);
        gl::enable(gl::kC_CullFace);
        gl::cullFace(gl::kF_Front);
        m_override = &m_shaders[kS_Normal];

        for(GameObject *object : m_objects)
            object->render(ptt);

        m_override = nullptr;
    }
    

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
    gl::uniform1f(m_shaders[kS_Skybox].getUniformLocation("u_Exposure"), m_exposure);
    gl::uniform1f(m_shaders[kS_Skybox].getUniformLocation("u_BloomThreshold"), m_bloomThreshold);
    m_skybox.draw();
    UIShader::unbind();
    popMatrix();

    //Rendu des objets
    gl::depthMask(true);
    gl::enable(gl::kC_DepthTest);
    gl::enable(gl::kC_CullFace);
    gl::cullFace(gl::kF_Back);

    UIShader &mainShdr = m_shaders[m_internalRefraction ? kS_Main : kS_MainNoRT];
    use3DShader(mainShdr);
    gl::uniform3f(mainShdr.getUniformLocation("u_CamPos"), camPos.x(), camPos.y(), camPos.z());
    gl::uniform1f(mainShdr.getUniformLocation("u_Exposure"), m_exposure);
    gl::uniform1f(mainShdr.getUniformLocation("u_BloomThreshold"), m_bloomThreshold);
    UIShader::unbind();

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

    if(m_lensFlareEnable && m_sunPosWorldSpace.length2() > 0.0f) {
        //Position et visibilite du soleil pour le lens flare
        float w = 1.0f;
        m::Vector3f sunPosScreenSpace((m_proj * m_view).multiplyEx(m_sunPosWorldSpace, w));
        m::Vector2f sunPosViewport(sunPosScreenSpace.xy() / w);

        if(sunPosViewport.x() >= -1.0f && sunPosViewport.x() <= 1.0f && sunPosViewport.y() >= -1.0f && sunPosViewport.y() <= 1.0f && sunPosScreenSpace.z() > 0.0f) {
            sunPosViewport.setY(-sunPosViewport.y());
            sunPosViewport = (sunPosViewport + 1.0f) * 0.5f;
            sunPosViewport *= m::Vector2<uint32_t>(m_ww, m_wh).cast<float>();

            m_sunPos = sunPosViewport.cast<int>();

            float visMultiplier = 1.0f;
            int minCoord = m::math::minimum(m_sunPos.x(), m_sunPos.y());
            int maxCoord = m::math::maximum(m_sunPos.x() - static_cast<int>(m_ww) + JJ_LENS_FLARE_MEASURE, m_sunPos.y() - static_cast<int>(m_wh) + JJ_LENS_FLARE_MEASURE);

            if(minCoord < JJ_LENS_FLARE_MEASURE)
                visMultiplier = static_cast<float>(minCoord) / static_cast<float>(JJ_LENS_FLARE_MEASURE);
            else if(maxCoord > 0)
                visMultiplier = 1.0f - static_cast<float>(maxCoord) / static_cast<float>(JJ_LENS_FLARE_MEASURE);

            const int bufX = m::math::clamp(m_sunPos.x() - JJ_LENS_FLARE_MEASURE / 2, 0, static_cast<int>(m_ww) - JJ_LENS_FLARE_MEASURE);
            const int bufY = m::math::clamp(static_cast<int>(m_wh) - (m_sunPos.y() - JJ_LENS_FLARE_MEASURE / 2) - 1, 0, static_cast<int>(m_wh) - JJ_LENS_FLARE_MEASURE);

            gl::bindBuffer(gl::kBT_PixelPackBuffer, m_PBOs[m_curPBO]);
            gl::readPixels(bufX, bufY, JJ_LENS_FLARE_MEASURE, JJ_LENS_FLARE_MEASURE, gl::kTF_DepthComponent, gl::kDT_Float, nullptr);
            gl::bindBuffer(gl::kBT_PixelPackBuffer, 0);
            m_curPBO = (m_curPBO + 1) % 2;

            //La profondeur a une trame de retard, mais c'est pas trop grave...
            gl::bindBuffer(gl::kBT_PixelPackBuffer, m_PBOs[m_curPBO]);
            float *ptrDepth = static_cast<float *>(gl::mapBuffer(gl::kBT_PixelPackBuffer, gl::kBA_ReadOnly));
            int visibleCount = 0;

            for(int i = 0; i < JJ_LENS_FLARE_MEASURE * JJ_LENS_FLARE_MEASURE; i++) {
                if(ptrDepth[i] >= 0.9999f)
                    visibleCount++;
            }

            m_sunVisibility = static_cast<float>(visibleCount) * visMultiplier / static_cast<float>(JJ_LENS_FLARE_MEASURE * JJ_LENS_FLARE_MEASURE);
            gl::unmapBuffer(gl::kBT_PixelPackBuffer);
            gl::bindBuffer(gl::kBT_PixelPackBuffer, 0);
        } else
            m_sunVisibility = 0.0f;
    } else
        m_sunVisibility = 0.0f;

    
    //Calcul de l'histogramme
    m_histo->compute(m_hdrFBO0.colorAttachmentID());


    /***************************** RENDU DES EFFETS *****************************/
    gl::disable(gl::kC_DepthTest);
    gl::depthMask(false);
    gl::disable(gl::kC_Blend);
    gl::disable(gl::kC_CullFace);

    if(m_bloomEnable) {
        //BLOOM: Reduction de la taille
        m_2Dmat.loadIdentity();
        m_bloomFBO[0].bindForRender();
        m_shaders[kS_NoOp].bind();
        gl::uniformMatrix3fv(m_shaders[kS_NoOp].getUniformLocation("u_Matrix"), 1, false, m_2Dmat.data());
        gl::bindTexture(gl::kTT_Texture2D, m_hdrFBO0.colorAttachmentID(1));
        gl::bindVertexArray(m_peVAO);
        gl::drawArrays(gl::kDM_Triangles, 0, 6);
        gl::bindVertexArray(0);
        gl::bindTexture(gl::kTT_Texture2D, 0);
        UIShader::unbind();

        //BLOOM: Flou
        for(int i = 0; i < 2; i++) {
            UIShader &shdr = m_shaders[(i == 0) ? kS_BlurX : kS_BlurY];

            m_bloomFBO[1 - i].bindForRender();
            shdr.bind();
            gl::uniform2f(shdr.getUniformLocation("u_InvTexSize"), m_halfInvTexSize.x(), m_halfInvTexSize.y());
            gl::bindTexture(gl::kTT_Texture2D, m_bloomFBO[i].colorAttachmentID());
            gl::bindVertexArray(m_peVAO);
            gl::drawArrays(gl::kDM_Triangles, 0, 6);
            gl::bindVertexArray(0);
            gl::bindTexture(gl::kTT_Texture2D, 0);
            UIShader::unbind();
        }
    } else {
        m_bloomFBO[0].bindForRender();
        gl::clear(gl::kCF_ColorBuffer);
    }

    if(m_fxaaEnable)
        m_sdrFBO.bindForRender();
    else
        Framebuffer::unbindFromRender(m_ww, m_wh);

    //Ajout du bloom + Tone mapping
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
    UIShader::unbind();

    if(m_fxaaEnable) {
        //FXAA
        Framebuffer::unbindFromRender(m_ww, m_wh);

        m_shaders[kS_FXAA].bind();
        gl::uniform2f(m_shaders[kS_FXAA].getUniformLocation("u_InvTexSize"), m_invTexSize.x(), m_invTexSize.y());
        gl::bindTexture(gl::kTT_Texture2D, m_sdrFBO.colorAttachmentID());
        gl::bindVertexArray(m_peVAO);
        gl::drawArrays(gl::kDM_Triangles, 0, 6);
        gl::bindVertexArray(0);
        gl::bindTexture(gl::kTT_Texture2D, 0);
        UIShader::unbind();
    }

    //Lens flare
    if(m_sunVisibility > 0.0f)
        renderLensFlare();

    //Affichage des buffers internes
    if(m_doDebugDraw) {
        m_2Dmat.loadIdentity();
        m_2Dmat.scale(0.25f);
        m_2Dmat.translate(3.0f, -3.0f);

        m_shaders[kS_NoOp].bind();
        debugDrawTexture(m_normalPass.colorAttachmentID());
        m_2Dmat.translate(-2.0f, 0.0f);
        debugDrawTexture(m_normalPass.depthAttachmentID());
        m_2Dmat.translate(-2.0f, 0.0f);
        debugDrawTexture(m_bloomFBO[0].colorAttachmentID());
        UIShader::unbind();
    }
}

void MainApp::debugDrawTexture(GLuint tex)
{
    gl::uniformMatrix3fv(m_shaders[kS_NoOp].getUniformLocation("u_Matrix"), 1, false, m_2Dmat.data());
    gl::bindTexture(gl::kTT_Texture2D, tex);
    gl::bindVertexArray(m_peVAO);
    gl::drawArrays(gl::kDM_Triangles, 0, 6);
    gl::bindVertexArray(0);
    gl::bindTexture(gl::kTT_Texture2D, 0);
}

void MainApp::grabMouse(bool grabbed)
{
    if(grabbed && !m_relativeMouse) {
        m_relativeMouse = true;
        m_lastCursorPosX = static_cast<double>(m_ww) * 0.5;
        m_lastCursorPosY = static_cast<double>(m_wh) * 0.5;

        glfwGetCursorPos(m_wnd, &m_lastCursorPosX, &m_lastCursorPosY);
        glfwSetInputMode(m_wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        uiCore.setCursorShown(false);

        if(m_camera != nullptr)
            m_camera->activate();
    } else if(m_relativeMouse) {
        m_relativeMouse = false;
        glfwSetInputMode(m_wnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        uiCore.setCursorShown(true);

        if(m_camera != nullptr)
            m_camera->deactivate();
    }
}

bool MainApp::onSliderValueChanged(UIElement *e)
{
    float val = static_cast<UISlider *>(e)->value();
    const m::String &name = e->name();
    Gem *gem = static_cast<Gem *>(m_objects[0]);

    if(name == "sIOR")
        gem->setIOR(val);
    else if(name == "sTemporalAdaptation")
        m_histo->setTemporalAdaptationFactor(std::exp(val));
    else if(name == "sBloomThreshold")
        m_bloomThreshold = val;
    else if(name == "sFOV") {
        m_proj = m::Matrix4f::perspective(val * static_cast<float>(M_PI) / 180.0f,
                                          static_cast<float>(m_ww) / static_cast<float>(m_wh),
                                          0.1f, 10.0f);
    } else if(name == "sCamSpeed")
        m_orbitalCam->setSpeed(val);

    return false;
}

bool MainApp::changeColor(UIElement *e)
{
    UIContainer *c = static_cast<UIContainer*>(e->parent());
    float r, g, b;

    try {
        r = c->byName<UISlider>("sCR")->value();
        g = c->byName<UISlider>("sCG")->value();
        b = c->byName<UISlider>("sCB")->value();
    } catch(UIUnknownElementException &ex) {
        return false;
    }

    static_cast<Gem*>(m_objects[0])->changeColor(r, g, b);
    return false;
}

bool MainApp::changeShape(UIElement *e)
{
    UIContainer *c = static_cast<UIContainer *>(e->parent());
    int n;
    float h, r, dh, dr;

    try {
        n  = c->byName<UISlider>("sN" )->intValue();
        h  = c->byName<UISlider>("sH" )->value();
        r  = c->byName<UISlider>("sR" )->value();
        dh = c->byName<UISlider>("sDH")->value();
        dr = c->byName<UISlider>("sDR")->value();
    } catch(UIUnknownElementException &ex) {
        return false;
    }

    const float y1 = h + dh;
    const float r1 = r - r * dr;

    static_cast<Gem*>(m_objects[0])->generate(n, h, r, y1, r1);
    return false;
}

bool MainApp::onCheckboxValueChanged(UIElement *e)
{
    bool val = static_cast<UICheckBox *>(e)->isChecked();
    const m::String &name = e->name();

    if(name == "cbRefraction")
        m_internalRefraction = val;
    else if(name == "cbBloom")
        m_bloomEnable = val;
    else if(name == "cbFXAA")
        m_fxaaEnable = val;
    else if(name == "cbLimitFPS")
        m_renderPeriod = val ? (1000.0 / 60.0) : 0.0;
    else if(name == "cbVSync")
        glfwSwapInterval(val ? 1 : 0);
    else if(name == "cbInfos")
        m_displayDebugString = val;
    else if(name == "cbDebugDraw")
        m_doDebugDraw = val;
    else if(name == "cbWireframe")
        m_useWireframe = val;
    else if(name == "cbLensFlare")
        m_lensFlareEnable = val;

    return false;
}

void MainApp::renderHUD()
{
    UIVStreamer &vs = uiCore.vertexStreamer();

    gl::enable(gl::kC_Blend);
    gl::blendFunc(gl::kBM_SrcAlpha, gl::kBM_OneMinusSrcAlpha);

    if(m_displayDebugString) {
        //Infos de debug
        vs.drawString(10.0f, 10.0f, m_font, m_debugString);

        //Fond des deux graphiques
        vs.begin(gl::kDM_Triangles, false);
        vs.quad(10, 70, 128, 64).quadColor(0, 0, 0, 200);
        vs.quad(10, 144, 128, 64).quadColor(0, 0, 0, 200);
        vs.draw();


        //Histogramme
        float histoMax = 0.0f;
        for(int i = 0; i < HistogramSize; i++) {
            if(m_histo->value(i) > histoMax)
                histoMax = m_histo->value(i);
        }

        histoMax = 64.0f / histoMax;
        vs.begin(gl::kDM_LineStrip, false);

        for(int i = 0; i < HistogramSize; i++) {
            float x = static_cast<float>(i * 2 + 10);
            float y = static_cast<float>(70 + 64) - m_histo->value(i) * histoMax;

            vs.vertexf(x, y).color(255, 255, 255);
        }

        vs.draw();


        //Graphique de numero de frame
        int curFrame = m_histo->dispatchPos() - 2; //-1 car Histogram::compute() a deja ete appele ET -1 car le temps de rendu a une frame de retard
        while(curFrame < 0)
            curFrame = m_histo->dispatchCount() + curFrame;

        vs.begin(gl::kDM_LineStrip, false);
        vs.vertexf(63 * 2 + 10, static_cast<float>(144 + 64 - curFrame * 4)).color(0, 255, 50);

        for(int i = 32 - 2; i >= 0; i--) {
            float x = static_cast<float>(i * 4 + 10);
            vs.vertexf(x, static_cast<float>(144 + 64 - curFrame * 4)).color(0, 255, 50);

            if(--curFrame < 0)
                curFrame = m_histo->dispatchCount() + curFrame;

            vs.vertexf(x, static_cast<float>(144 + 64 - curFrame * 4)).color(0, 255, 50);
        }

        vs.draw();


        //Graphique de temps de rendu
        int ftMin = 0x7FFFFFFF;
        int ftMax = 0;

        for(int i = 0; i < ~m_frameTimeGraph; i++) {
            const int ft = m_frameTimeGraph[i];

            if(ft < ftMin)
                ftMin = ft;

            if(ft > ftMax)
                ftMax = ft;
        }
        
        const float ftMul = 48.0f / static_cast<float>(ftMax - ftMin);
        vs.begin(gl::kDM_LineStrip, false);

        for(int i = 0; i < ~m_frameTimeGraph; i++) {
            float x = static_cast<float>(i * 4 + 10);
            float y = static_cast<float>(144 + 48) - static_cast<float>(m_frameTimeGraph[~m_frameTimeGraph - i - 1] - ftMin) * ftMul;

            vs.vertexf(x, y).color(255, 255, 255);
        }

        vs.draw();
    }

    if(m_skybox.isDoingAsyncOp()) {
        m::String str("CHARGEMENT...");
        int strY0, strY1, strW;
        m_font->boundingBox(str, strY0, strY1, strW);

        int px = (static_cast<int>(m_ww) - strW - 10) / 2;
        int py = (static_cast<int>(m_wh) - (strY1 - strY0) - 10) / 2;

        vs.begin(gl::kDM_TriangleStrip, false);
        vs.quad(px, py, strW + 10, (strY1 - strY0) + 10);
        vs.quadColor(0, 0, 0, 128);
        vs.draw();

        vs.drawString(static_cast<float>(px + 5), static_cast<float>(py + 5 - m_font->lineHeight() - strY0), m_font, str);
    }

    gl::disable(gl::kC_Blend);
}

typedef struct
{
    float l;          //Position sur l'axe
    float size;       //Taille du sprite
    uint8_t color[4]; //Couleur du sprite
    float v;          //Numero du sprite (position verticale dans la texture)
} LensFlareEntry;

/* Tableau de rendu de lens flare */
static LensFlareEntry g_lfEntries[] = {
    { 0.7f, 56.2f , { 170, 255, 0  , 65 }, 1.0f },
    { 3.1f, 198.6f, { 255, 203, 0  , 70 }, 0.0f },
    { 0.6f, 50.8f , { 84 , 152, 255, 45 }, 0.0f },
    { 3.3f, 213.1f, { 84 , 152, 255, 69 }, 3.0f },
    { 1.3f, 91.9f , { 84 , 152, 255, 50 }, 1.0f },
    { 2.5f, 160.5f, { 170, 255, 0  , 69 }, 0.0f },
    { 2.6f, 171.4f, { 84 , 152, 255, 71 }, 3.0f },
    { 2.2f, 150.8f, { 255, 84 , 226, 32 }, 3.0f },
    { 0.9f, 179.5f, { 255, 255, 255, 75 }, 2.0f },
    { 2.7f, 177.4f, { 170, 255, 0  , 30 }, 0.0f },
    { 1.7f, 115.6f, { 255, 203, 0  , 39 }, 1.0f },
    { 0.5f, 128.2f, { 255, 255, 255, 75 }, 3.0f },
    { 2.5f, 83.2f , { 255, 203, 0  , 45 }, 1.0f },
    { 2.8f, 55.5f , { 84 , 152, 255, 44 }, 3.0f },
    { 0.6f, 46.5f , { 255, 203, 0  , 68 }, 0.0f },
    { 2.6f, 171.1f, { 255, 84 , 226, 44 }, 3.0f },
    { 2.8f, 183.9f, { 255, 255, 255, 75 }, 2.0f },
    { 2.0f, 47.2f , { 84 , 152, 255, 44 }, 3.0f },
    { 3.7f, 249.2f, { 84 , 152, 255, 58 }, 0.0f },
    { 1.0f, 72.1f , { 255, 255, 255, 75 }, 3.0f }
};

void MainApp::renderLensFlare()
{
    m::Vector2f sunPos(m_sunPos.cast<float>());
    m::Vector2f dirVec(m::Vector2<uint32_t>(m_ww, m_wh).cast<float>() * 0.5f - sunPos);
    UIVStreamer &vs = uiCore.vertexStreamer();

    gl::enable(gl::kC_Blend);
    gl::blendFunc(gl::kBM_SrcAlpha, gl::kBM_OneMinusSrcAlpha);

    gl::bindTexture(gl::kTT_Texture2D, m_lensFlareSprite);
    vs.begin(gl::kDM_Triangles, true);

    for(int i = 0; i < sizeof(g_lfEntries) / sizeof(LensFlareEntry); i++) {
        LensFlareEntry &entry = g_lfEntries[i];
        m::Vector2f pos = sunPos + dirVec * entry.l - entry.size * 0.5f;
        float alpha = static_cast<float>(entry.color[3]) * m_sunVisibility;

        vs.quadf(pos, entry.size);
        vs.quadTexf(0.0f, entry.v * 0.25f, 1.0f, (entry.v + 1.0f) * 0.25f);
        vs.quadColor(entry.color[0], entry.color[1], entry.color[2], static_cast<uint8_t>(alpha));
    }

    vs.draw();
    gl::bindTexture(gl::kTT_Texture2D, 0);

    gl::disable(gl::kC_Blend);
}

bool MainApp::onChangeSkyboxClicked(UIElement *e)
{
    m::File skyboxRoot("textures");
    int sid = m_curSkybox;

    for(int i = 0; i < m_skyboxData.size() - 1; i++) {
        sid = (sid + 1) % m_skyboxData.size();
        m::File fle(skyboxRoot, m_skyboxData[sid]["filename"].asString());

        if(fle.exists() && m_skybox.loadAsync(fle.path())) {
            m_curSkybox = sid;
            m_skyboxBtn->setDisabled();
            break;
        }
    }

    return false;
}

void MainApp::use3DShader(UIShader &shdr)
{
    //Envoi des matrices au shader
    shdr.bind();
    gl::uniformMatrix4fv(shdr.getUniformLocation("u_Projection"), 1, false, m_proj.data());
    gl::uniformMatrix4fv(shdr.getUniformLocation("u_View"), 1, false, m_view.data());
    gl::uniformMatrix4fv(shdr.getUniformLocation("u_Model"), 1, false, m_model[m_curModelMat].data());
}

void MainApp::handleMouseButtonEvent(int button, int action, int mods)
{
    bool hitUI = uiCore.handleMouseButtonEvent(button, action, mods);

    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        if(action == GLFW_PRESS && !hitUI)
            grabMouse(true);
        else if(action == GLFW_RELEASE)
            grabMouse(false);
    }
}

void MainApp::handleMouseMotionEvent(float dx, float dy)
{
    if(m_camera != nullptr && m_relativeMouse)
        m_camera->onMouseMove(dx, dy);
}

void MainApp::handleKeyboardEvent(int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS) {
        if(key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(m_wnd, GLFW_TRUE);
        else if(key == GLFW_KEY_P)
            m_displayStats = !m_displayStats;
        else if(key == GLFW_KEY_F1 && m_viewWindow != nullptr)
            uiCore.addWindow(m_viewWindow);
        else if(key == GLFW_KEY_F2 && m_customizeWindow != nullptr)
            uiCore.addWindow(m_customizeWindow);
        else if(key == GLFW_KEY_C) {
            //Changement de camera

            if(m_camera == m_freeCam)
                m_camera = m_orbitalCam;
            else
                m_camera = m_freeCam;
        } else
            m_camera->onKeyDown(scancode);

        uiCore.handleKeyDownEvent(key, scancode, mods);
    } else if(action == GLFW_RELEASE) {
        m_camera->onKeyUp(scancode);
        uiCore.handleKeyUpEvent(key, scancode, mods);
    }
}

void MainApp::handleMouseScrollEvent(float amntX, float amntY)
{
    if(m_camera != nullptr)
        m_camera->onScroll(amntY);
}
