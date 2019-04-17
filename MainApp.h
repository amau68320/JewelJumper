#pragma once
#include <mgpcl/List.h>
#include <mgpcl/Time.h>
#include <GLFW/glfw3.h>
#include "GameObject.h"

class MainApp
{
public:
	MainApp(GLFWwindow* wnd);
	virtual ~MainApp();
	void run();

private:
	m::List<GameObject*> m_objects;
	double m_currentTimeUpdate;
	double m_currentTimeRenderer;
	int m_fps;
	GLFWwindow* m_wnd;
};

