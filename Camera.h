#pragma once
#include <mgpcl/Matrix4.h>

/*
 * Une interface de base dont toutes les cameras doivent heriter
 */
class Camera
{
public:
    virtual ~Camera();

    /*
     * Met a jour mat et camPos selon la camera representee par cette classe.
     * ptt est le temps de tick partiel (0-1).
     */
    virtual void getTransform(m::Matrix4f &mat, m::Vector3f &camPos, float ptt) = 0;

    /*
     * Appele ~20x par secondes. dt est le temps en secondes depuis le dernier appel.
     */
    virtual void update(float dt);

    /*
     * Appele lorsque la position de la souris a bouge.
     * dx est la difference entre la nouvelle position X et l'ancienne.
     * dy est la difference entre la nouvelle position Y et l'ancienne.
     */
    virtual void onMouseMove(float dx, float dy);

    /*
     * Appele lorsqu'une touche est appuyee.
     * scancode est le scancode de la touche appuyee.
     */
    virtual void onKeyDown(int scancode);

    /*
     * Appele lorsqu'une touche est relachee.
     * scancode est le scancode de la touche relachee.
     */
    virtual void onKeyUp(int scancode);

    /*
     * Appele lorsque l'utilisateur prends le controle de cette camera
     * en gardant le clic gauche enfoncee, en dehors de l'IHM.
     */
    virtual void activate();

    /*
     * Appele lorsque l'utilisateur relache le controle de cette camera,
     * c'est a dire en relachant le clic gauche.
     */
    virtual void deactivate();

    /*
     * Appele lorsque l'utilisateur fait bouger la roulette de sa souris.
     */
    virtual void onScroll(float amnt);
};

/*
 * Une camera que l'utilisateur peut deplacer comme bon lui semble.
 */
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

/*
 * Une camera orbitale qui, par defaut, tourne autour du centre de la scene (0, 0, 0).
 * L'utilisateur peut en prendre le controle pour changer l'altitude, la latitude et
 * la longitude (mais pas la position directement, comme avec FreeCamera).
 */
class RotatingCamera : public Camera
{
public:
    RotatingCamera();

    void getTransform(m::Matrix4f &mat, m::Vector3f &camPos, float ptt) override;
    void activate() override;
    void onMouseMove(float dx, float dy) override;
    void onScroll(float amnt) override;
    void deactivate() override;

    /*
     * Permet de changer la vitesse de rotation lorsque la camera est en mode automatique.
     */
    void setSpeed(float spd);

    /*
     * Retourne la vitesse de rotation utilisee en mode automatique.
     */
    float getSpeed() const
    {
        return m_speed;
    }

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
