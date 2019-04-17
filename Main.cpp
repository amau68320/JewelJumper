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
    m::Logger::setLoggerInstance(new m::BasicLogger);

    mlogger.info(M_LOG, "Initializing JewelJumper...");
    m::time::initTime();
    m::inet::initialize();

    //Read config
    m::SimpleConfig cfg("jewel_jumper.ini");
    if(cfg.load() != m::kCLE_None && cfg.lastError() != m::kCLE_FileNotFound) {
        m::String err(cfg.errorString());
        mlogger.error(M_LOG, "Could not load jewel_jumper.ini: %s", err.raw());
        return 1;
    }

    const int ww = cfg["window"]["width"].asInt(1024);
    const int wh = cfg["window"]["height"].asInt(576);
    const bool useNetLogger = cfg["logging"]["useNetLogger"].asBool();
    const bool autoStartNL = cfg["logging"]["autoStartNetLogger"].asBool();
    const m::String &nlAddr = cfg["logging"]["netLoggerAddress"].value("127.0.0.1");
    cfg.save();

    if(useNetLogger) {
        m::NetLogger *nl = new m::NetLogger;
        if(autoStartNL) {
            if(nl->tryAutoStartSubprocess(true))
                m::time::sleepMs(1000);
            else
                mlogger.warning(M_LOG, "Could not start NetLogger!");
        }

        if(nl->connect(nlAddr)) {
            delete m::Logger::setLoggerInstance(nl);


        } else {
            mlogger.warning(M_LOG, "Could not connect to NetLogger; continuing with the basic logger...");
            delete nl;
        }
    }

    //Setup SDL
    if(glfwInit() == GLFW_FALSE) {
        mlogger.error(M_LOG, "Could not initialize GLFW!");
        return 2;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *wnd = glfwCreateWindow(ww, wh, "Jewel Jumper", nullptr, nullptr);
    if(wnd == nullptr) {
        mlogger.error(M_LOG, "Could not create window!");
        glfwTerminate();
        return 3;
    }

    glfwMakeContextCurrent(wnd);

    const int maj = glfwGetWindowAttrib(wnd, GLFW_CONTEXT_VERSION_MAJOR);
    const int min = glfwGetWindowAttrib(wnd, GLFW_CONTEXT_VERSION_MINOR);
    const int profile = glfwGetWindowAttrib(wnd, GLFW_OPENGL_PROFILE);

    if(maj != 3 || min != 3)
        mlogger.warning(M_LOG, "Using OpenGL %d.%d", maj, min);

    if(profile != GLFW_OPENGL_CORE_PROFILE)
        mlogger.warning(M_LOG, "OpenGL profile is NOT core.");

    GLenum glewErr = glewInit();
    if(glewErr != GLEW_OK) {
        mlogger.error(M_LOG, "Could initialize glew: %s (error %d)", glewGetErrorString(glewErr), glewErr);
        glfwDestroyWindow(wnd);
        glfwTerminate();
        return 6;
    }

    int fbw, fbh;
    glfwGetFramebufferSize(wnd, &fbw, &fbh);

    //Parse args
    m::ProgramArgs pargs(argc, const_cast<const char **>(argv));

    //Jewel Jumper main loop
	MainApp app(wnd);
	app.run();

    mlogger.info(M_LOG, "Shutting down, goodbye...");
    glfwDestroyWindow(wnd);
    glfwTerminate();
    delete m::Logger::instance();
	m::inet::release();

    return 0;
}
