#include "Gem.h"
#include "MainApp.h"

Gem::Gem() : m_vbo(0), m_ebo(0), m_vao(0), m_numIndices(0), m_ior(1.45f),
             m_numVertices(0), m_color{ 1.0f, 0.0f, 0.0f, 1.0f }
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
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);

        float x = std::cos(a0) * r0;
        float z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);
        a0 += step;

        x = std::cos(a0) * r0;
        z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);
    }

    //Triangles inferieurs
    float a1 = step * 0.5f;
    a0 = 0.0f;

    for(int i = 0; i < numSides; i++) {
        float x = std::cos(a0) * r0;
        float z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);
        a0 += step;

        x = std::cos(a1) * r1;
        z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);
        a1 += step;

        x = std::cos(a0) * r0;
        z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);
    }

    //fillColor(0.0f, 1.0f, 0.0f);

    //Triangles superieurs
    a0 = 0.0f;
    a1 = step * -0.5f;

    for(int i = 0; i < numSides; i++) {
        float x = std::cos(a1) * r1;
        float z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1 + 2);
        a1 += step;

        x = std::cos(a0) * r0;
        z = std::sin(a0) * r0;
        m_vertices << GemVertex(x, y0, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);
        a0 += step;

        x = std::cos(a1) * r1;
        z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1 - 2);
    }

    //fillColor(0.0f, 0.0f, 1.0f);

    //Face du dessus
    m_vertices << GemVertex(0.0f, y1, 0.0f);
    const uint16_t center = static_cast<uint16_t>(m_vertices.size() - 1);

    a1 = step * -0.5f;
    for(int i = 0; i < numSides; i++) {
        float x = std::cos(a1) * r1;
        float z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);
        a1 += step;

        m_indices << center;

        x = std::cos(a1) * r1;
        z = std::sin(a1) * r1;
        m_vertices << GemVertex(x, y1, z);
        m_indices  << static_cast<uint16_t>(m_vertices.size() - 1);
    }

    for(GemVertex &v: m_vertices) {
        v.color[0] = m_color[0];
        v.color[1] = m_color[1];
        v.color[2] = m_color[2];
        v.color[3] = m_color[3];
    }

    m_numIndices = static_cast<GLsizei>(m_indices.size());

    for(int i = 0; i < m_indices.size(); i += 3) {
        GemVertex &v0 = m_vertices[m_indices[i + 0]];
        GemVertex &v1 = m_vertices[m_indices[i + 1]];
        GemVertex &v2 = m_vertices[m_indices[i + 2]];

        m::Vector3f a(v1.pos - v0.pos);
        m::Vector3f b(v2.pos - v0.pos);
        m::Vector3f n(a.cross(b).normalized());

        v0.normal = n;
        v1.normal = n;
        v2.normal = n;
    }

    if(m_vbo == 0)
        m_vbo = gl::genBuffer();

    gl::bindBuffer(gl::kBT_ArrayBuffer, m_vbo);
    gl::bufferData(gl::kBT_ArrayBuffer, m_vertices.size() * sizeof(GemVertex), m_vertices.begin(), gl::kBU_StaticDraw);
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);
    m_numVertices = m_vertices.size();
    m_vertices.clear();

    if(m_ebo == 0)
        m_ebo = gl::genBuffer();

    gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_ebo);
    gl::bufferData(gl::kBT_ElementArrayBuffer, m_indices.size() * sizeof(uint16_t), m_indices.begin(), gl::kBU_StaticDraw);
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);
    m_indices.clear();

    if(m_vao == 0)
        m_vao = gl::genVertexArray();

    gl::bindVertexArray(m_vao);
    gl::bindBuffer(gl::kBT_ArrayBuffer, m_vbo);
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_ebo);
    gl::enableVertexAttribArray(0); //Position
    gl::enableVertexAttribArray(1); //Couleur
    gl::enableVertexAttribArray(2); //Normale
    gl::vertexAttribPointer(0, 3, gl::kDT_Float, false, sizeof(GemVertex), reinterpret_cast<void*>(offsetof(GemVertex, pos)));
    gl::vertexAttribPointer(1, 4, gl::kDT_Float, false, sizeof(GemVertex), reinterpret_cast<void*>(offsetof(GemVertex, color)));
    gl::vertexAttribPointer(2, 3, gl::kDT_Float, false, sizeof(GemVertex), reinterpret_cast<void*>(offsetof(GemVertex, normal)));
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);
    gl::bindVertexArray(0);
}

void Gem::render(float ptt)
{
    UIShader &shdr = MainApp::instance().mainShader();
    MainApp::instance().use3DShader(shdr);

    GLint iorUniformLoc = shdr.getUniformLocation("u_IOR");
    if(iorUniformLoc != -1)
        gl::uniform1f(iorUniformLoc, m_ior);

    MainApp::instance().skybox().bindCubeMap();
    gl::bindVertexArray(m_vao);
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, m_ebo);
    gl::drawElements(gl::kDM_Triangles, m_numIndices, gl::kDT_UnsignedShort, nullptr);
    gl::bindBuffer(gl::kBT_ElementArrayBuffer, 0);
    gl::bindVertexArray(0);
    Skybox::unbindCubeMap();
    UIShader::unbind();
}

void Gem::changeColor(float r, float g, float b, float a)
{
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
    m_color[3] = a;

    if(m_vbo == 0)
        return;

    gl::bindBuffer(gl::kBT_ArrayBuffer, m_vbo);
    GemVertex *data = static_cast<GemVertex *>(gl::mapBuffer(gl::kBT_ArrayBuffer, gl::kBA_ReadWrite));

    for(int i = 0; i < m_numVertices; i++) {
        data[i].color[0] = r;
        data[i].color[1] = g;
        data[i].color[2] = b;
        data[i].color[3] = a;
    }

    gl::unmapBuffer(gl::kBT_ArrayBuffer);
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);
}

