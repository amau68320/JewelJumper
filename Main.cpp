#include <mgpcl/SimpleConfig.h>
#include <mgpcl/BasicLogger.h>
#include <mgpcl/Time.h>
#include <mgpcl/INet.h>
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include "MainApp.h"

#ifdef MGPCL_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "resource.h"
#endif

int main(int argc, char *argv[])
{
    //Init MGPCL
    m::Logger::setLoggerInstance(new m::BasicLogger);

    mlogger.info(M_LOG, "Initialisation de JewelJumper...");
    m::time::initTime();
    m::inet::initialize();
    m::inet::initSSL();

    //Lecture de la config INI
    m::SimpleConfig cfg("gem_studio.ini");
    if(cfg.load() != m::kCLE_None && cfg.lastError() != m::kCLE_FileNotFound) {
        m::String err(cfg.errorString());
        mlogger.error(M_LOG, "Impossible de lire jewel_jumper.ini: %s", err.raw());
        return 1;
    }

    int ww = cfg["window"]["width"].asInt(0);
    int wh = cfg["window"]["height"].asInt(0);
    const bool allocConsole = cfg["misc"]["allocConsole"].asBool();
    const int histoFirstDivider = m::math::maximum(0, cfg["misc"]["histogramFirstDivider"].asInt(3));
    cfg.save();

#if defined(MGPCL_WIN) && !defined(_DEBUG)
    if(allocConsole) {
        AllocConsole();
        delete m::Logger::setLoggerInstance(new m::BasicLogger);
    }
#endif

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
    int monX, monY, monW, monH;
    glfwGetMonitorWorkarea(mainMon, &monX, &monY, &monW, &monH);

    if(monW <= 0 || monH <= 0) {
        monW = 1366;
        monH = 768;
    }

    if(ww <= 0 && wh <= 0) {
        ww = monW * 7 / 8;
        wh = monH * 7 / 8;
    } else if(ww <= 0)
        ww = wh * 16 / 9;
    else if(wh <= 0)
        wh = ww * 9 / 16;

    int min = 6;
    GLFWwindow *wnd = nullptr;

    while(wnd == nullptr && min >= 3) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, min);

        wnd = glfwCreateWindow(ww, wh, "GemStudio - Copyright (C) 2019 A. Einholtz & N. Barbotin - IN55 P2019", nullptr, nullptr);
        min--;
    }

    if(wnd == nullptr) {
        mlogger.error(M_LOG, "Impossible de creer la fenetre !");
        glfwTerminate();
        return 3;
    }

#ifdef MGPCL_WIN
    HWND hWnd = glfwGetWin32Window(wnd);
    HICON hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1));
    SendMessage(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
#endif

    glfwSetWindowPos(wnd, monX + (monW - ww) / 2, monY + (monH - wh) / 2);
    glfwMakeContextCurrent(wnd);
    glfwSwapInterval(1);

    //On verifie que la version est bien celle demandee
    int maj = glfwGetWindowAttrib(wnd, GLFW_CONTEXT_VERSION_MAJOR);
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

    if(app.setup(fbw, fbh, static_cast<GLuint>(histoFirstDivider))) {
        app.run();
        app.cleanup();
    } else
        mlogger.error(M_LOG, "Erreur lors de l'initialisation de MainApp ! Impossible de continuer...");

    //Fin
    glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    mlogger.info(M_LOG, "Liberation des ressources...");
    glfwDestroyWindow(wnd);
    glfwTerminate();
    delete m::Logger::instance();
	m::inet::release();

#if defined(MGPCL_WIN)
    if(allocConsole)
        system("pause");
#endif

    return 0;
}

#if defined(MGPCL_WIN) && !defined(_DEBUG)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    return main(__argc, __argv);
}

#endif
