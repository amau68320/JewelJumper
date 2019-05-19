#pragma once
#include "GameObject.h"
#include <aiso/GL.h>
#include <mgpcl/Vector3.h>
#include <mgpcl/List.h>

class GemVertex
{
public:
    GemVertex(float x, float y, float z) : pos(x, y, z)
    {
    }

    m::Vector3f pos;
    float color[4];
    m::Vector3f normal;
    m::Vector3f tangent;
};

class Gem : public GameObject
{
public:
    Gem();
    ~Gem();

    /*        r0
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
    void render(float ptt) override;

    void setIOR(float ior)
    {
        m_ior = ior;
    }

    float ior() const
    {
        return m_ior;
    }

private:
    void fillColor(float r, float g, float b, float a = 1.0f);

    m::List<GemVertex> m_vertices;
    m::List<uint16_t> m_indices;
    GLuint m_vbo;
    GLuint m_ebo;
    GLuint m_vao;
    GLsizei m_numIndices;
    int m_lastColor;
    float m_ior;
};
