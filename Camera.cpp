#include "Camera.h"
#include <GLFW/glfw3.h>
#include <mgpcl/Time.h>

#define PIF static_cast<float>(M_PI)

Camera::~Camera()
{
}

void Camera::update(float dt)
{
}

void Camera::onMouseMove(float dx, float dy)
{
}

void Camera::onKeyDown(int scancode)
{
}

void Camera::onKeyUp(int scancode)
{
}

void Camera::activate()
{
}

void Camera::deactivate()
{
}

void Camera::onScroll(float amnt)
{
}

FreeCamera::FreeCamera() : m_pos(1.0f, 1.5f, -3.0f), m_keyStates(0)
{
    m_keyBindings[kCK_Up   ] = glfwGetKeyScancode(GLFW_KEY_W);
    m_keyBindings[kCK_Right] = glfwGetKeyScancode(GLFW_KEY_D);
    m_keyBindings[kCK_Down ] = glfwGetKeyScancode(GLFW_KEY_S);
    m_keyBindings[kCK_Left ] = glfwGetKeyScancode(GLFW_KEY_A);
    m_keyBindings[kCK_Fast ] = glfwGetKeyScancode(GLFW_KEY_LEFT_SHIFT);

    m_oldPos = m_pos;
    m_dir    = -m_pos.normalized();
    m_strafe = (m_dir ^ m::Vector3f(0.0f, 1.0f, 0.0f)).normalized();
    recoverAngles();
}

void FreeCamera::getTransform(m::Matrix4f &mat, m::Vector3f &camPos, float ptt)
{
    camPos = m_oldPos * (1.0f - ptt) + m_pos * ptt;

    mat.loadIdentity();
    mat.lookAt(camPos, camPos + m_dir, m::Vector3f(0.0f, 1.0f, 0.0f));
}

void FreeCamera::update(float dt)
{
    float fw = 0.0f;
    float strafe = 0.0f;
    float speed = dt * 4.0f;

    if(isKeyDown(kCK_Up))
        fw += 1.0f;

    if(isKeyDown(kCK_Down))
        fw -= 1.0f;

    if(isKeyDown(kCK_Left))
        strafe -= 1.0f;

    if(isKeyDown(kCK_Right))
        strafe += 1.0f;

    if(isKeyDown(kCK_Fast))
        speed *= 2.0f;

    m_oldPos = m_pos;

    if(fw != 0.0f || strafe != 0.0f) {
        m::Vector3f dir(m_dir * fw + m_strafe * strafe);
        dir.normalize();

        m_pos += dir * speed;
    }
}

void FreeCamera::onMouseMove(float dx, float dy)
{
    m_theta += dx * -0.01f;
    m_phi += dy * 0.01f;

    if(m_phi <= 0.0f)
        m_phi = 0.01f;
    else if(m_phi >= PIF)
        m_phi = PIF - 0.01f;

    const float sinPhi = std::sin(m_phi);
    m_dir.setX(sinPhi * std::sin(m_theta));
    m_dir.setY(std::cos(m_phi));
    m_dir.setZ(sinPhi * std::cos(m_theta));

    m_strafe = (m_dir ^ m::Vector3f(0.0f, 1.0f, 0.0f)).normalized();
}

void FreeCamera::onKeyDown(int scancode)
{
    for(uint32_t i = 0; i < kCK_Count; i++) {
        if(scancode == m_keyBindings[i]) {
            m_keyStates |= (1U << i);
            break;
        }
    }
}

void FreeCamera::onKeyUp(int scancode)
{
    for(uint32_t i = 0; i < kCK_Count; i++) {
        if(scancode == m_keyBindings[i]) {
            m_keyStates &= ~(1U << i);
            break;
        }
    }
}

void FreeCamera::recoverAngles()
{
    m_phi = std::acos(m_dir.y());

    const float sinPhi = std::sin(m_phi);
    m_theta = std::atan2(m_dir.x() / sinPhi, m_dir.z() / sinPhi);
}

void FreeCamera::deactivate()
{
    m_keyStates = 0U;
}

RotatingCamera::RotatingCamera() : m_speed(0.5f), m_theta(0.0f), m_phi(0.0f), m_auto(true), m_transitioning(false),
                                   m_transitionTime(0.0), m_oldPhi(0.0f), m_oldR(2.5f), m_newR(2.5f), m_rTime(0.0),
                                   m_changingR(false)
{
    m_timeOffset = m::time::getTimeMs();
}

void RotatingCamera::getTransform(m::Matrix4f &mat, m::Vector3f &camPos, float ptt)
{
    double t = m::time::getTimeMs();

    if(m_auto) {
        m_theta = std::fmod(static_cast<float>((t - m_timeOffset) / 1000.0) * m_speed, 4.0f * PIF);
        m_phi = (std::sin(m_theta * 0.5f) + 1.0f) * 0.5f;

        m_phi *= 4.0f * PIF / 6.0f;
        m_phi += PIF / 6.0f;

        if(m_transitioning) {
            float lerp = static_cast<float>((t - m_transitionTime) / 1000.0);
            if(lerp >= 1.0f) {
                m_transitioning = false;
                lerp = 1.0f;
            }

            lerp = ((-2.0f * lerp + 3.0f) * lerp) * lerp;
            m_phi = (1.0f - lerp) * m_oldPhi + lerp * m_phi;
        }
    }

    const float sinPhi = std::sin(m_phi);
    camPos.setX(sinPhi * std::sin(m_theta));
    camPos.setY(std::cos(m_phi));
    camPos.setZ(sinPhi * std::cos(m_theta));
    camPos *= computedR(t);

    mat.loadIdentity();
    mat.lookAt(camPos, m::Vector3f(0.0f, 0.0f, 0.0f), m::Vector3f(0.0f, 1.0f, 0.0f));
}

void RotatingCamera::setSpeed(float spd)
{
    double r = static_cast<double>(m_speed / spd);
    m_speed = spd;
    m_timeOffset = m::time::getTimeMs() * (1.0 - r) + m_timeOffset * r;
}

void RotatingCamera::activate()
{
    m_auto = false;
}

void RotatingCamera::onMouseMove(float dx, float dy)
{
    m_theta += dx * -0.01f;
    m_phi += dy * -0.01f;

    if(m_phi <= 0.0f)
        m_phi = 0.01f;
    else if(m_phi >= PIF)
        m_phi = PIF - 0.01f;
}

void RotatingCamera::onScroll(float amnt)
{
    double t = m::time::getTimeMs();

    m_oldR = computedR(t);
    m_newR = m::math::clamp(m_newR + amnt * -0.5f, 2.0f, 5.0f);
    m_changingR = true;
    m_rTime = t;
}

void RotatingCamera::deactivate()
{
    const double t = m::time::getTimeMs();

    m_auto = true;
    m_transitioning = true;
    m_transitionTime = t;
    m_oldPhi = m_phi;
    m_timeOffset = t - m_theta * 1000.0 / m_speed;
}

float RotatingCamera::computedR(double t)
{
    if(!m_changingR)
        return m_newR;

    float lerp = static_cast<float>((t - m_rTime) / 250.0);
    if(lerp >= 1.0f) {
        m_changingR = false;
        lerp = 1.0f;
    }

    lerp = ((-2.0f * lerp + 3.0f) * lerp) * lerp;
    return (1.0f - lerp) * m_oldR + lerp * m_newR;
}
