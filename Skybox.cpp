#include "Skybox.h"
#include "RGBE.h"

Skybox::Skybox() : m_tex(0), m_vbo(0), m_vao(0), m_cubemap(0)
{
}

Skybox::~Skybox()
{
    if(m_vao != 0)
        gl::deleteVertexArray(m_vao);

    if(m_vbo != 0)
        gl::deleteBuffer(m_vbo);

    if(m_tex != 0)
        gl::deleteTexture(m_tex);

    if(m_cubemap != 0)
        gl::deleteTexture(m_cubemap);
}

void myTexImage(gl::TextureTarget target, int x, int y, float *data)
{
    gl::pixelStorei(gl::kPSP_UnpackSkipPixels, x * 512);
    gl::pixelStorei(gl::kPSP_UnpackSkipRows, y * 512);
    gl::texImage2D(target, 0, gl::kTF_RGB16F, 512, 512, 0, gl::kTF_RGB, gl::kDT_Float, data);
}

static void rotate90(float *data, int imgW, int i, int j)
{
    float *tmp = new float[512 * 512 * 3];
    for(int y = 0; y < 512; y++)
        m::mem::copy(tmp + y * 512 * 3, data + ((j * 512 + y) * imgW + i * 512) * 3, 512 * 3 * sizeof(float));

    for(int y = 0; y < 512; y++) {
        for(int x = 0; x < 512; x++) {
            float *src = tmp + (y * 512 + x) * 3;
            float *dst = data + ((j * 512 + x) * imgW + i * 512 + (511 - y)) * 3;

            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
        }
    }

    delete[] tmp;
}

bool Skybox::load(const m::String &fname)
{
    FILE *fle = nullptr;
    fopen_s(&fle, fname.raw(), "rb");
    int imgW, imgH;

    if(RGBE_ReadHeader(fle, &imgW, &imgH, nullptr) != RGBE_RETURN_SUCCESS) {
        fclose(fle);
        return false;
    }

    float *data = new float[imgW * imgH * 3];
    if(RGBE_ReadPixels(fle, data, imgW * imgH) != RGBE_RETURN_SUCCESS) {
        delete[] data;
        fclose(fle);
        return false;
    }

    fclose(fle);
    if(m_tex != 0)
        gl::deleteTexture(m_tex);

    m_tex = gl::genTexture();
    gl::bindTexture(gl::kTT_Texture2D, m_tex);
    gl::texImage2D(gl::kTT_Texture2D, 0, gl::kTF_RGB16F, imgW, imgH, 0, gl::kTF_RGB, gl::kDT_Float, data);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MagFilter, gl::kTF_Linear);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MinFilter, gl::kTF_Linear);
    gl::bindTexture(gl::kTT_Texture2D, 0);

    //Flemme lvl 9999999999999 d'ecrire rotate270
    rotate90(data, imgW, 1, 1);
    rotate90(data, imgW, 1, 1);
    rotate90(data, imgW, 1, 1);

    rotate90(data, imgW, 2, 1);

    if(m_cubemap != 0)
        gl::deleteTexture(m_cubemap);

    m_cubemap = gl::genTexture();
    gl::bindTexture(gl::kTT_TextureCubeMap, m_cubemap);
    gl::pixelStorei(gl::kPSP_UnpackRowLength, imgW);
    myTexImage(gl::kTT_TextureCubeMapNZ, 0, 1, data);
    myTexImage(gl::kTT_TextureCubeMapPZ, 1, 0, data);
    myTexImage(gl::kTT_TextureCubeMapPX, 2, 0, data);
    myTexImage(gl::kTT_TextureCubeMapNX, 0, 0, data);
    myTexImage(gl::kTT_TextureCubeMapNY, 2, 1, data);
    myTexImage(gl::kTT_TextureCubeMapPY, 1, 1, data);
    gl::pixelStorei(gl::kPSP_UnpackSkipPixels, 0);
    gl::pixelStorei(gl::kPSP_UnpackSkipRows, 0);
    gl::pixelStorei(gl::kPSP_UnpackRowLength, 0);
    gl::generateMipmap(gl::kTT_TextureCubeMap);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MagFilter, gl::kTF_Linear);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MinFilter, gl::kTF_LinearMipmapLinear);
    gl::bindTexture(gl::kTT_TextureCubeMap, 0);
    delete[] data;

    float vec[] = {
        -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f,    1.0f,  1.0f, -1.0f, //Z- => 0,1
        -1.0f, -1.0f, -1.0f,   -1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,    1.0f, -1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //Z+ => 1,0
        -1.0f, -1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f, -1.0f,   -1.0f,  1.0f, -1.0f,   -1.0f,  1.0f,  1.0f, //X- => 2,0
        -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,    1.0f,  1.0f, -1.0f,    1.0f,  1.0f,  1.0f, //X+ => 0,0
         1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,    1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f, //Y- => 2,1
        -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,    1.0f,  1.0f,  1.0f, //Y+ => 1,1
        -1.0f,  1.0f, -1.0f,   -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f
    };

    const float trd1 = 1.0f / 3.0f;
    const float trd2 = 2.0f / 3.0f;
    const float ox = 0.5f / (3.0f * 512.0f);
    const float oy = 0.5f / (2.0f * 512.0f);

    //0,0    1,0    1,1
    //0,0    0,1    1,1

    float color[] = {
            trd1 - ox, 1.0f - oy,   0.0f + ox, 1.0f - oy,   0.0f + ox, 0.5f + oy,
            trd1 - ox, 1.0f - oy,   trd1 - ox, 0.5f + oy,   0.0f + ox, 0.5f + oy,

            trd1 + ox, 0.5f - oy,   trd2 - ox, 0.5f - oy,   trd2 - ox, 0.0f + oy,
            trd1 + ox, 0.5f - oy,   trd1 + ox, 0.0f + oy,   trd2 - ox, 0.0f + oy,

            0.0f + ox, 0.5f - oy,   0.0f + ox, 0.0f + oy,   trd1 - ox, 0.0f + oy,
            0.0f + ox, 0.5f - oy,   trd1 - ox, 0.5f - oy,   trd1 - ox, 0.0f + oy,

            1.0f - ox, 0.5f - oy,   1.0f - ox, 0.0f + oy,   trd2 + ox, 0.0f + oy,
            1.0f - ox, 0.5f - oy,   trd2 + ox, 0.5f - oy,   trd2 + ox, 0.0f + oy,

            1.0f - ox, 1.0f - oy,   1.0f - ox, 0.5f + oy,   trd2 + ox, 0.5f + oy,
            1.0f - ox, 1.0f - oy,   trd2 + ox, 1.0f - oy,   trd2 + ox, 0.5f + oy,

            trd2 - ox, 0.5f + oy,   trd2 - ox, 1.0f - oy,   trd1 + ox, 1.0f - oy,
            trd2 - ox, 0.5f + oy,   trd1 + ox, 0.5f + oy,   trd1 + ox, 1.0f - oy,
    };

    if(m_vbo != 0)
        gl::deleteBuffer(m_vbo);

    m_vbo = gl::genBuffer();
    gl::bindBuffer(gl::kBT_ArrayBuffer, m_vbo);
    gl::bufferData(gl::kBT_ArrayBuffer, sizeof(vec) + sizeof(color), nullptr, gl::kBU_StaticDraw);
    gl::bufferSubData(gl::kBT_ArrayBuffer, 0, sizeof(vec), vec);
    gl::bufferSubData(gl::kBT_ArrayBuffer, sizeof(vec), sizeof(color), color);
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);

    if(m_vao != 0)
        gl::deleteVertexArray(m_vao);

    m_vao = gl::genVertexArray();
    gl::bindVertexArray(m_vao);
    gl::bindBuffer(gl::kBT_ArrayBuffer, m_vbo);
    gl::enableVertexAttribArray(0);
    gl::enableVertexAttribArray(1);
    gl::vertexAttribPointer(0, 3, gl::kDT_Float, false, 0, reinterpret_cast<void *>(0));
    gl::vertexAttribPointer(1, 2, gl::kDT_Float, false, 0, reinterpret_cast<void *>(sizeof(vec)));
    gl::bindBuffer(gl::kBT_ArrayBuffer, 0);
    gl::bindVertexArray(0);

    return true;
}

void Skybox::draw()
{
    gl::bindTexture(gl::kTT_Texture2D, m_tex);
    gl::bindVertexArray(m_vao);
    gl::drawArrays(gl::kDM_Triangles, 0, 36);
    gl::bindVertexArray(0);
    gl::bindTexture(gl::kTT_Texture2D, 0);
}
