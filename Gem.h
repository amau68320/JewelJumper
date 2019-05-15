#pragma once
#include "GameObject.h"
#include <mgpcl/Vector3.h>
#include <mgpcl/List.h>

typedef struct
{
    m::Vector3f pos;
    m::Vector3f normal;
    m::Vector3f tangent;
    float color[4];
    m::Vector2f texCoord;
} GemVertex;

class Gem : public GameObject
{
public:
    Gem();
    ~Gem();

    /*
     *   ____________           ^
     *  /            \          |
     *  \------------/     ^    |
     *   \          /      |    |
     *    \        /       |    |
     *     \      /     y0 |    | y1
     *      \    /         |    |
     *       \  /          |    |
     *        \/           -    -
     */
    void generate(int numSides, float y0, float y1);

private:
    m::List<GemVertex> m_vertices;
};
