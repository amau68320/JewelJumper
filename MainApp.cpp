#include "MainApp.h"

#define UPDATE_INTERVAL 20

MainApp::MainApp(GLFWwindow* wnd) : m_objects(), m_currentTimeUpdate(0.0), m_currentTimeRenderer(0.0), m_fps(0)
{
	m_wnd = wnd;
}

MainApp::~MainApp(){}

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
			for (GameObject* object : m_objects)
			{
				object->update(m::time::getTimeMs() - tmpUpdate);
			}
		}

		m_currentTimeRenderer = m::time::getTimeMs();

		glDepthMask(GL_TRUE);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (GameObject* object : m_objects)
		{
			object->renderer((m::time::getTimeMs() - m_currentTimeUpdate) / UPDATE_INTERVAL);
		}

		glfwSwapBuffers(m_wnd);

		waitTime = min((1000.0 / UPDATE_INTERVAL) - (m::time::getTimeMs() - m_currentTimeUpdate),
					   (1000.0 / m_fps) - (m::time::getTimeMs() - m_currentTimeRenderer));
		
		if (waitTime >= 5)
			m::time::sleepMs(static_cast<uint32_t>(waitTime));
	}
}