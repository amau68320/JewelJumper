#include <mgpcl/SimpleConfig.h>
#include <mgpcl/BasicLogger.h>
#include <mgpcl/NetLogger.h>
#include <mgpcl/ProgramArgs.h>
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

    const int ww = cfg["window"]["width"].asInt(1024);
    const int wh = cfg["window"]["height"].asInt(576);
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *wnd = glfwCreateWindow(ww, wh, "THE JEWEL EDITOR - Copyright (C) 2019 A. Einholtz & N. Barbotin - IN55 P2019", nullptr, nullptr);
    if(wnd == nullptr) {
        mlogger.error(M_LOG, "Impossible de creer la fenetre !");
        glfwTerminate();
        return 3;
    }

    glfwMakeContextCurrent(wnd);

    //On verifie que la version est bien celle demandee
    const int maj = glfwGetWindowAttrib(wnd, GLFW_CONTEXT_VERSION_MAJOR);
    const int min = glfwGetWindowAttrib(wnd, GLFW_CONTEXT_VERSION_MINOR);
    const int profile = glfwGetWindowAttrib(wnd, GLFW_OPENGL_PROFILE);

    if(maj != 4 || min != 6)
        mlogger.warning(M_LOG, "Version d'OpenGL: %d.%d", maj, min);

    if(profile != GLFW_OPENGL_CORE_PROFILE)
        mlogger.warning(M_LOG, "Le profil d'OpenGL utilise n'est pas \"core\" !");

    GLenum glewErr = glewInit();
    if(glewErr != GLEW_OK) {
        mlogger.error(M_LOG, "Impossible d'initialiser glew: %s (erreur %d)", glewGetErrorString(glewErr), glewErr);
        glfwDestroyWindow(wnd);
        glfwTerminate();
        return 6;
    }

    int fbw, fbh;
    glfwGetFramebufferSize(wnd, &fbw, &fbh);

    //Lecture des arguments de commande
    m::ProgramArgs pargs(argc, const_cast<const char **>(argv));

    //Boucle principale de Jewel Jumper
	MainApp app(wnd);

    if(app.setup(pargs, fbw, fbh))
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
