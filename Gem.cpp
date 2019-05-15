#include "Gem.h"

Gem::Gem() : m_vbo(0), m_ebo(0), m_vao(0), m_numIndices(0)
{
}

Gem::~Gem()
{
    if(m_vbo != 0)
        gl::deleteBuffer(m_vbo);

    if(m_ebo != 0)
        gl::deleteBuffer(m_ebo);

    if(m_vao != 0)
        gl::deleteVertexArray(m_vao);
}

void Gem::generate(int numSides, float y0, float r0, float y1, float r1)
{
    float a0 = 0.0f;
    const float step = 2.0f * static_cast<float>(M_PI) / static_cast<float>(numSides);

    //Cotes
    for(int i = 0; i < numSides; i++) {
        m_vertices << GemVertex(0.0f, 0.0f, 0.0f);
        m_indices  << static_cast<uint16_t>(m_vertices.size());

        float x = std::cos(a0) * r0;
        float z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
        a0 += step;

        x = std::cos(a0) * r0;
        z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
    }

    //Triangles inferieurs
    float a1 = step * 0.5f;
    a0 = 0.0f;

    for(int i = 0; i < numSides; i++) {
        float x = std::cos(a0) * r0;
        float z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
        a0 += step;

        x = std::cos(a1) * r1;
        z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
        a1 += step;

        x = std::cos(a0) * r0;
        z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
    }

    //Triangles superieurs
    a0 = 0.0f;
    a1 = step * -0.5f;

    for(int i = 0; i < numSides; i++) {
        float x = std::cos(a1) * r1;
        float z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
        a1 += step;

        x = std::cos(a0) * r0;
        z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
        a0 += step;

        x = std::cos(a1) * r1;
        z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
    }

    //Face du dessus
    m_vertices << GemVertex(0.0f, y1, 0.0f);
    const uint16_t center = static_cast<uint16_t>(m_vertices.size());

    a1 = step * -0.5f;
    for(int i = 0; i < numSides; i++) {
        float x = std::cos(a1) * r1;
        float z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
        a1 += step;

        m_indices << center;

        x = std::cos(a1) * r1;
        z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size());
    }

    m_numIndices = static_cast<GLsizei>(m_indices.size());

    if(m_vbo != 0)
        gl::deleteBuffer(m_vbo);

    m_vbo = gl::genBuffer();
    gl::bindBuffer(gl::kBT_ArrayBuffer, m_vbo);
    gl::bufferData(gl::kBT_ArrayBuffer, m_vertices.size() * sizeof(GemVertex), m_vertices.begin(), gl::kBU_StaticDraw);
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);
    m_vertices.clear();

    if(m_ebo != 0)
        gl::deleteBuffer(m_ebo);

    m_ebo = gl::genBuffer();
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_ebo);
    gl::bufferData(gl::kBT_ElementArrayBuffer, m_indices.size() * sizeof(uint16_t), m_indices.begin(), gl::kBU_StaticDraw);
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);
    m_indices.clear();

    if(m_vao != 0)
        gl::deleteVertexArray(m_vao);

    m_vao = gl::genVertexArray();
    gl::bindVertexArray(m_vao);
    gl::bindBuffer(gl::kBT_ArrayBuffer, m_vbo);
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_ebo);
    gl::enableVertexAttribArray(0); //Position
    gl::enableVertexAttribArray(1); //Normal
    gl::enableVertexAttribArray(2); //Tangent
    gl::enableVertexAttribArray(3); //Color
    gl::vertexAttribPointer(0, 3, gl::kDT_Float, false, sizeof(GemVertex), reinterpret_cast<void*>(offsetof(GemVertex, pos)));
    gl::vertexAttribPointer(1, 3, gl::kDT_Float, false, sizeof(GemVertex), reinterpret_cast<void*>(offsetof(GemVertex, normal)));
    gl::vertexAttribPointer(2, 3, gl::kDT_Float, false, sizeof(GemVertex), reinterpret_cast<void*>(offsetof(GemVertex, tangent)));
    gl::vertexAttribPointer(3, 4, gl::kDT_Float, false, sizeof(GemVertex), reinterpret_cast<void*>(offsetof(GemVertex, color)));
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);
    gl::bindVertexArray(0);
}

void Gem::render(float ptt)
{
    gl::bindVertexArray(m_vao);
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_ebo);
    gl::drawElements(gl::kDM_Triangles, m_numIndices, gl::kDT_UnsignedShort, nullptr);
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);
    gl::bindVertexArray(0);
}
