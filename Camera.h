#pragma once
#include <mgpcl/Matrix4.h>

class Camera
{
public:
    virtual ~Camera();
    virtual void getTransform(m::Matrix4f &mat, m::Vector3f &camPos, float ptt) = 0;
    virtual void update(float dt);
    virtual void onMouseMove(float dx, float dy);
    virtual void onKeyDown(int scancode);
    virtual void onKeyUp(int scancode);
    virtual void activate();
    virtual void deactivate();
    virtual void onScroll(float amnt);
};

class FreeCamera : public Camera
{
public:
    FreeCamera();
    void getTransform(m::Matrix4f &mat, m::Vector3f &camPos, float ptt) override;
    void update(float dt) override;
    void onMouseMove(float dx, float dy) override;
    void onKeyDown(int scancode) override;
    void onKeyUp(int scancode) override;
    void deactivate() override;

private:
    enum CameraKey
    {
        kCK_Up,
        kCK_Right,
        kCK_Down,
        kCK_Left,
        kCK_Fast,

        kCK_Count
    };

    bool isKeyDown(CameraKey k) const
    {
        return (m_keyStates & (1U << k)) != 0;
    }

    void recoverAngles();

    m::Vector3f m_pos;
    m::Vector3f m_dir;
    m::Vector3f m_strafe;
    m::Vector3f m_oldPos;

    float m_theta;
    float m_phi;
    uint32_t m_keyStates;
    int m_keyBindings[kCK_Count];
};

class RotatingCamera : public Camera
{
public:
    RotatingCamera();

    void getTransform(m::Matrix4f &mat, m::Vector3f &camPos, float ptt) override;
    void setSpeed(float spd);

    void activate() override;
    void onMouseMove(float dx, float dy);
    void onScroll(float amnt);
    void deactivate() override;

private:
    float computedR(double t);

    double m_timeOffset;
    float m_speed;
    float m_theta;
    float m_phi;
    bool m_auto;
    bool m_transitioning;
    double m_transitionTime;
    float m_oldPhi;

    float m_oldR;
    float m_newR;
    double m_rTime;
    bool m_changingR;
};
