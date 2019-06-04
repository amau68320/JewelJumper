#pragma once
#include <aiso/GL.h>
#include <mgpcl/List.h>

enum
{
    HistogramSize = 64,
    HistogramNumBuffers = 2,
    HistogramWorkgroupSize = 16,
    HistogramNumShaders = 3,
    HistogramNumITexs = 3
};

/*
 * Classe utilitaire permettant de calculer un histogramme de
 * la scene et de gerer l'exposition automatique.
 */
class Histogram
{
public:
    Histogram();
    ~Histogram();

    /*
     * Initialise l'histogramme et alloue les textures necessaires en fonction
     * de la taille de l'ecran passee en parameteres (w et h). Le parametre
     * histoDiv0 est un peu technique, mais une bonne valeur pour ce parametre
     * est 3.
     *
     * Retourne false en cas d'erreur.
     */
    bool setup(GLuint w, GLuint h, GLuint histoDiv0);

    /*
     * Calcul un histogramme d'indice de lumination pour la texture HDR passee
     * en parametre. Celle-ci doit absolument etre au format RGBA16F.
     *
     * Le calcul est relativement long et se fait sur environ 4 trames, mais
     * cette classe gere ca en interne et compute() doit etre appele a chaque
     * trame.
     *
     * Une fois le calcul de l'histogramme termine, une exposition adaptee est
     * determinee. Celle-ci peut etre obtenu via le getter 'computedExposure()'.
     */
    void compute(GLuint color);

    /*
     * Accede a la valeur normalisee de l'histogramme (entre 0 et 1). L'indice
     * de lumination associe a idx est calcule de la maniere suivante:
     * EV100(idx) = idx / 1.96875 - 16
     *
     * Cette methode est surtout utilisee pour du debogage.
     */
    float value(int idx) const
    {
        return static_cast<float>(m_histo1[idx]) / static_cast<float>(m_ww * m_wh);
    }

    /*
     * Retourne un valeur d'exposition adaptee a la scene actuelle
     */
    float computedExposure() const
    {
        float t = static_cast<float>(m_dispatchPos) / static_cast<float>(m_dispatchCount);
        return (1.0f - t) * m_oldAutoExposure + t * m_autoExposure;
    }

    /*
     * Retourne le facteur de vitesse d'adapation a la lumination
     */
    float temporalAdaptationFactor() const
    {
        return m_tau;
    }

    /*
     * Change le facteur de vitesse d'adaptation a la lumination
     */
    void setTemporalAdaptationFactor(float tau)
    {
        m_tau = tau;
    }

    /*
     * Retourne l'avancement du calcul de l'histogramme. Cette valeur
     * est comprise dans l'intervalle [0; dispatchCount()[
     *
     * Cette methode est surtout utilisee pour du debogage.
     */
    int dispatchPos() const
    {
        return m_dispatchPos;
    }

    /*
     * Retourne le nombre d'etapes du calcul de l'histogramme. Cette
     * valeur vaut generalement 4.
     *
     * Cette methode est surtout utilisee pour du debogage.
     */
    int dispatchCount() const
    {
        return m_dispatchCount;
    }

private:
    class ITex
    {
    public:
        ITex(GLuint _id, GLuint _w, GLuint _h) : id(_id), w(_w), h(_h)
        {
        }

        GLuint workgroupsX() const
        {
            return (w + HistogramWorkgroupSize - 1) / HistogramWorkgroupSize;
        }

        GLuint workgroupsY() const
        {
            return (h + HistogramWorkgroupSize - 1) / HistogramWorkgroupSize;
        }

        GLuint id;
        GLuint w;
        GLuint h;
    };

    bool loadShader(int id, const char *fname, GLuint dispatch0Mult);

    GLuint m_shader[HistogramNumShaders];
    GLuint m_program[HistogramNumShaders];

    GLuint m_ssbo[HistogramNumBuffers];
    int m_curBuf;

    m::List<ITex> m_interTexs;
    GLuint m_texs[HistogramNumITexs];

    GLuint m_histo1[HistogramSize];
    GLuint m_histo2[HistogramSize];
    GLuint m_ww;
    GLuint m_wh;

    float m_oldAutoExposure;
    float m_autoExposure;
    float m_temporalL;
    float m_tau;
    double m_lastTime;

    int m_dispatchPos;
    int m_dispatchMarks[4];
    int m_dispatchCount;
};
