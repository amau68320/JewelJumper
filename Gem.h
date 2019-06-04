#pragma once
#include "GameObject.h"
#include <aiso/GL.h>
#include <mgpcl/Vector3.h>
#include <mgpcl/List.h>

/*
 * Objet du jeu representant une gemme.
 */
class Gem : public GameObject
{
public:
    Gem();
    ~Gem();
    void render(float ptt) override;

    /*
     * Genere de maniere procedurale la forme de cette gemme. 'numSides'
     * correspond au nombre de cotes de cette gemme. Il doit etre au 
     * minimum egal a 3. Quand aux autres parametres, se referer aux
     * schema ci-dessous:
     *
     *        r0
     * <-------------->
     *
     *        r1
     *    <-------->
     *   ____________           ^
     *  /\/\/\/\/\/\/\          |
     *  \------------/     ^    |
     *   \          /      |    |
     *    \        /       |    |
     *     \      /     y0 |    | y1
     *      \    /         |    |
     *       \  /          |    |
     *        \/           -    -
     */
    void generate(int numSides, float y0, float r0, float y1, float r1);

    /*
     * Change la couleur de cette gemme.
     */
    void changeColor(float r, float g, float b, float a = 1.0f);

    /*
     * Change l'indice de refraction de cette gemme.
     */
    void setIOR(float ior)
    {
        m_ior = ior;
    }

    /*
     * Retourne l'indice de refraction de cette gemme.
     */
    float ior() const
    {
        return m_ior;
    }

private:
    class GemVertex
    {
    public:
        GemVertex(float x, float y, float z) : pos(x, y, z)
        {
        }

        m::Vector3f pos;
        float color[4];
        m::Vector3f normal;
    };

    m::List<GemVertex> m_vertices;
    m::List<uint16_t> m_indices;
    GLuint m_vbo;
    GLuint m_ebo;
    GLuint m_vao;
    GLsizei m_numIndices;
    float m_ior;
    int m_numVertices;
    float m_color[4];
};
