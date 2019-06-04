#pragma once
#include <aiso/GL.h>
#include <mgpcl/String.h>
#include <mgpcl/Thread.h>
#include <mgpcl/Atomic.h>
#include <cstdio>

/*
 * Classe representant une skybox HDR et sa cubemap associee,
 * et gerant le chargement (a)synchrone de celle-ci.
 */
class Skybox
{
public:
    Skybox();
    ~Skybox();

    /*
     * Charge une skybox HDR a partir de son fichier de texture.
     * Cette fonction est bloquante et peut prendre du temps.
     * Elle retourne false en cas d'erreur.
     */
    bool load(const m::String &fname);

    /*
     * Affiche cette skybox a l'ecran
     */
    void draw();

    /*
     * Charge une skybox HDR a partir de son fichier de texture.
     * 
     * Il s'agit d'une alternative a load(fname) qui ne fait
     * qu'ouvrir le fichier passe en parametre, puis continue
     * le chargement dans un thread a part pour eviter de bloquer
     * le thread principal.
     *
     * Retourne false en cas d'erreur. Meme si cette methode
     * retourne true, le chargement peut encore echouer. Dans
     * ce cas, l'ancienne skybox sera gardee.
     */
    bool loadAsync(const m::String &fname);

    /*
     * Appele regulierement.
     * 
     * Permet surtout de voir si un chargement asynchrone
     * (initie par loadAsync(fname)) s'est termine et,
     * dans ce cas, supprimer l'ancienne skybox et la remplace
     * par celle qui vient d'etre chargee.
     */
    bool update();

    /*
     * Bind la cubemap de cette skybox
     */
    void bindCubeMap()
    {
        gl::bindTexture(gl::kTT_TextureCubeMap, m_cubemap);
    }

    /*
     * Unbind la cubemap actuelle
     */
    static void unbindCubeMap()
    {
        gl::bindTexture(gl::kTT_TextureCubeMap, 0);
    }

    /*
     * Retourne true si une texture est actuellement entrain
     * d'etre chargee dans un autre thread.
     */
    bool isDoingAsyncOp() const
    {
        return m_asyncOpInProgress;
    }

private:
    bool loadBlocking(FILE *fp);
    void uploadTextureData();

    GLuint m_tex;
    GLuint m_vbo;
    GLuint m_vao;
    GLuint m_cubemap;

    //Asynchrone
    int m_asyncW;
    int m_asyncH;
    float *m_skyboxData;
    float *m_cubemapData;
    bool m_asyncOpInProgress;
    m::Atomic m_asyncOpFinished;
    m::FunctionalThread *m_thread;
};
