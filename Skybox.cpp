#include "Skybox.h"
#include "RGBE.h"
#include <mgpcl/Logger.h>

class HDRImageData
{
public:
    float *data;
    int tileSz;
    int imgW;
};

Skybox::Skybox() : m_tex(0), m_vbo(0), m_vao(0), m_cubemap(0), m_asyncW(0), m_asyncH(0),
                   m_skyboxData(nullptr), m_cubemapData(nullptr), m_asyncOpInProgress(false),
                   m_thread(nullptr)
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

    if(m_skyboxData != nullptr)
        delete[] m_skyboxData;

    if(m_cubemapData != nullptr)
        delete[] m_cubemapData;

    if(m_thread != nullptr)
        delete m_thread;
}

void myTexImage(gl::TextureTarget target, int x, int y, HDRImageData &img)
{
    gl::pixelStorei(gl::kPSP_UnpackSkipPixels, x * img.tileSz);
    gl::pixelStorei(gl::kPSP_UnpackSkipRows, y * img.tileSz);
    gl::texImage2D(target, 0, gl::kTF_RGB16F, img.tileSz, img.tileSz, 0, gl::kTF_RGB, gl::kDT_Float, img.data);
}

static void rotate90(HDRImageData &img, int i, int j)
{
    float *tmp = new float[img.tileSz * img.tileSz * 3];
    for(int y = 0; y < img.tileSz; y++)
        m::mem::copy(tmp + y * img.tileSz * 3, img.data + ((j * img.tileSz + y) * img.imgW + i * img.tileSz) * 3, img.tileSz * 3 * sizeof(float));

    for(int y = 0; y < 512; y++) {
        for(int x = 0; x < 512; x++) {
            float *src = tmp + (y * img.tileSz + x) * 3;
            float *dst = img.data + ((j * img.tileSz + x) * img.imgW + i * img.tileSz + (img.tileSz - y - 1)) * 3;

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

    if(fle == nullptr)
        return false;

    if(RGBE_ReadHeader(fle, &m_asyncW, &m_asyncH, nullptr) != RGBE_RETURN_SUCCESS) {
        fclose(fle);
        return false;
    }

    if(!loadBlocking(fle))
        return false; //loadBlocking se charge de fermer le fichier

    uploadTextureData();

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

bool Skybox::loadBlocking(FILE *fp)
{
    const size_t numPixels = static_cast<size_t>(m_asyncW) * static_cast<size_t>(m_asyncH) * 3;

    m_skyboxData = new float[numPixels];
    if(RGBE_ReadPixels(fp, m_skyboxData, m_asyncW * m_asyncH) != RGBE_RETURN_SUCCESS) {
        delete[] m_skyboxData;
        m_skyboxData = nullptr;
        fclose(fp);

        return false;
    }

    fclose(fp);

    m_cubemapData = new float[numPixels];
    m::mem::copy(m_cubemapData, m_skyboxData, numPixels * sizeof(float));

    HDRImageData img;
    img.data = m_cubemapData;
    img.tileSz = m_asyncH / 2;
    img.imgW = m_asyncW;

    //Flemme lvl 9999999999999 d'ecrire rotate270
    rotate90(img, 1, 1);
    rotate90(img, 1, 1);
    rotate90(img, 1, 1);
    rotate90(img, 2, 1);

    return true;
}

void Skybox::uploadTextureData()
{
    if(m_tex != 0)
        gl::deleteTexture(m_tex);

    m_tex = gl::genTexture();
    gl::bindTexture(gl::kTT_Texture2D, m_tex);
    gl::texImage2D(gl::kTT_Texture2D, 0, gl::kTF_RGB16F, m_asyncW, m_asyncH, 0, gl::kTF_RGB, gl::kDT_Float, m_skyboxData);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MagFilter, gl::kTF_Linear);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MinFilter, gl::kTF_Linear);
    gl::bindTexture(gl::kTT_Texture2D, 0);

    delete[] m_skyboxData;
    m_skyboxData = nullptr;

    HDRImageData img;
    img.data = m_cubemapData;
    img.tileSz = m_asyncH / 2;
    img.imgW = m_asyncW;

    if(m_cubemap != 0)
        gl::deleteTexture(m_cubemap);

    m_cubemap = gl::genTexture();
    gl::bindTexture(gl::kTT_TextureCubeMap, m_cubemap);
    gl::pixelStorei(gl::kPSP_UnpackRowLength, m_asyncW);
    myTexImage(gl::kTT_TextureCubeMapNZ, 0, 1, img);
    myTexImage(gl::kTT_TextureCubeMapPZ, 1, 0, img);
    myTexImage(gl::kTT_TextureCubeMapPX, 2, 0, img);
    myTexImage(gl::kTT_TextureCubeMapNX, 0, 0, img);
    myTexImage(gl::kTT_TextureCubeMapNY, 2, 1, img);
    myTexImage(gl::kTT_TextureCubeMapPY, 1, 1, img);
    gl::pixelStorei(gl::kPSP_UnpackSkipPixels, 0);
    gl::pixelStorei(gl::kPSP_UnpackSkipRows, 0);
    gl::pixelStorei(gl::kPSP_UnpackRowLength, 0);
    gl::generateMipmap(gl::kTT_TextureCubeMap);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MagFilter, gl::kTF_Linear);
    gl::texParameteri(gl::kTT_Texture2D, gl::kTP_MinFilter, gl::kTF_LinearMipmapLinear);
    gl::bindTexture(gl::kTT_TextureCubeMap, 0);

    delete[] m_cubemapData;
    m_cubemapData = nullptr;
}

bool Skybox::loadAsync(const m::String &fname)
{
    mlogger.info(M_LOG, "Chargement de la skybox %s", fname.raw());

    if(m_asyncOpInProgress)
        return false;

    FILE *fle = nullptr;
    fopen_s(&fle, fname.raw(), "rb");

    if(fle == nullptr)
        return false;

    if(RGBE_ReadHeader(fle, &m_asyncW, &m_asyncH, nullptr) != RGBE_RETURN_SUCCESS) {
        fclose(fle);
        return false;
    }

    if(m_thread != nullptr)
        delete m_thread; //Ne devrait pas arriver

    m_asyncOpInProgress = true;
    m_thread = new m::FunctionalThread([this, fle] () {
        loadBlocking(fle);
        m_asyncOpFinished.set(1);
    }, "SB-LDR");

    m_thread->start();
    return true;
}

bool Skybox::update()
{
    if(m_asyncOpInProgress && m_asyncOpFinished.get()) {
        if(m_thread != nullptr) {
            m_thread->join();
            delete m_thread;
            m_thread = nullptr;
        }

        if(m_skyboxData == nullptr || m_cubemapData == nullptr)
            mlogger.error(M_LOG, "Erreur lors de la lecture asynchrone");
        else
            uploadTextureData();

        m_asyncOpFinished.set(0);
        m_asyncOpInProgress = false;
        return true;
    }

    return false;
}
