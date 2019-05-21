#include <mgpcl/SimpleConfig.h>
#include <mgpcl/BasicLogger.h>
#include <mgpcl/NetLogger.h>
#include <mgpcl/Time.h>
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include "MainApp.h"

int main(int argc, char *argv[])
{
    //Init MGPCL
    m::Logger::setLoggerInstance(new m::BasicLogger);

    mlogger.info(M_LOG, "Initialisation de JewelJumper...");
    m::time::initTime();
    m::inet::initialize();

    //Lecture de la config INI
    m::SimpleConfig cfg("jewel_jumper.ini");
    if(cfg.load() != m::kCLE_None && cfg.lastError() != m::kCLE_FileNotFound) {
        m::String err(cfg.errorString());
        mlogger.error(M_LOG, "Impossible de lire jewel_jumper.ini: %s", err.raw());
        return 1;
    }

    int ww = cfg["window"]["width"].asInt(0);
    int wh = cfg["window"]["height"].asInt(0);
    const bool useNetLogger = cfg["logging"]["useNetLogger"].asBool();
    const bool autoStartNL = cfg["logging"]["autoStartNetLogger"].asBool();
    const m::String &nlAddr = cfg["logging"]["netLoggerAddress"].value("127.0.0.1");
    cfg.save();

    //Prise en charge du NetLogger
    if(useNetLogger) {
        m::NetLogger *nl = new m::NetLogger;
        if(autoStartNL) {
            if(nl->tryAutoStartSubprocess(true))
                m::time::sleepMs(1000);
            else
                mlogger.warning(M_LOG, "Impossible de demarrer le NetLogger !");
        }

        if(nl->connect(nlAddr)) {
            delete m::Logger::setLoggerInstance(nl);


        } else {
            mlogger.warning(M_LOG, "Impossible de se connecter au NetLogger; reprise avec le logger basique...");
            delete nl;
        }
    }

    //Init de GLFW
    if(glfwInit() == GLFW_FALSE) {
        mlogger.error(M_LOG, "Impossible d'intialiser GLFW !");
        return 2;
    }

    //Config OpenGL & creation de la fenetre
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWmonitor *mainMon = glfwGetPrimaryMonitor();
    int monW, monH;
    glfwGetMonitorWorkarea(mainMon, nullptr, nullptr, &monW, &monH);

    if(monW <= 0 || monH <= 0) {
        monW = 1366;
        monH = 768;
    }

    if(ww <= 0 && wh <= 0) {
        ww = monW * 3 / 4;
        wh = monH * 3 / 4;
    } else if(ww <= 0)
        ww = wh * 16 / 9;
    else if(wh <= 0)
        wh = ww * 9 / 16;

    int maj = 4;
    int min = 6;
    GLFWwindow *wnd = nullptr;

    while(wnd == nullptr && (maj > 3 || min >= 3)) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, maj);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, min);

        wnd = glfwCreateWindow(ww, wh, "THE JEWEL EDITOR - Copyright (C) 2019 A. Einholtz & N. Barbotin - IN55 P2019", nullptr, nullptr);

        if(--min <= 0) {
            maj = 3;
            min = 3;
        }
    }

    if(wnd == nullptr) {
        mlogger.error(M_LOG, "Impossible de creer la fenetre !");
        glfwTerminate();
        return 3;
    }

    glfwMakeContextCurrent(wnd);

    //On verifie que la version est bien celle demandee
    maj = glfwGetWindowAttrib(wnd, GLFW_CONTEXT_VERSION_MAJOR);
    min = glfwGetWindowAttrib(wnd, GLFW_CONTEXT_VERSION_MINOR);
    const int profile = glfwGetWindowAttrib(wnd, GLFW_OPENGL_PROFILE);

    mlogger.info(M_LOG, "Version d'OpenGL: %d.%d %s", maj, min, (profile == GLFW_OPENGL_CORE_PROFILE) ? "CORE" : "COMPAT");

    GLenum glewErr = glewInit();
    if(glewErr != GLEW_OK) {
        mlogger.error(M_LOG, "Impossible d'initialiser glew: %s (erreur %d)", glewGetErrorString(glewErr), glewErr);
        glfwDestroyWindow(wnd);
        glfwTerminate();
        return 6;
    }

    mlogger.info(M_LOG, "Carte graphique: %s", gl::getRenderer());
    mlogger.info(M_LOG, "Vendeur: %s", gl::getVendor());

    int fbw, fbh;
    glfwGetFramebufferSize(wnd, &fbw, &fbh);

    //Boucle principale de Jewel Jumper
	MainApp app(wnd);

    if(app.setup(fbw, fbh))
        app.run();
    else
        mlogger.error(M_LOG, "Erreur lors de l'initialisation de MainApp ! Impossible de continuer...");

    //Fin
    glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    mlogger.info(M_LOG, "Liberation des ressources...");
    glfwDestroyWindow(wnd);
    glfwTerminate();
    delete m::Logger::instance();
	m::inet::release();
    //system("pause");

    return 0;
}
