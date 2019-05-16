#pragma once
#include <GL/glew.h>

namespace gl
{
    enum FramebufferTarget
    {
        kFBT_Framebuffer = GL_FRAMEBUFFER,
        kFBT_DrawFramebuffer = GL_DRAW_FRAMEBUFFER
    };

    enum TextureTarget
    {
        kTT_Texture2D = GL_TEXTURE_2D,
        kTT_Texture3D = GL_TEXTURE_3D,
        kTT_TextureCubeMap = GL_TEXTURE_CUBE_MAP,
        kTT_TextureCubeMapNX = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        kTT_TextureCubeMapNY = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        kTT_TextureCubeMapNZ = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        kTT_TextureCubeMapPX = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        kTT_TextureCubeMapPY = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        kTT_TextureCubeMapPZ = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        kTT_Texture2DArray = GL_TEXTURE_2D_ARRAY
    };

    enum BufferTarget
    {
        kBT_ArrayBuffer = GL_ARRAY_BUFFER,
        kBT_ElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER
    };

    enum BufferUsage
    {
        kBU_StaticDraw = GL_STATIC_DRAW,
        kBU_DynamicDraw = GL_DYNAMIC_DRAW,
        kBU_StreamDraw = GL_STREAM_DRAW
    };

    enum BufferAccess
    {
        kBA_ReadOnly = GL_READ_ONLY,
        kBA_WriteOnly = GL_WRITE_ONLY,
        kBA_ReadWrite = GL_READ_WRITE
    };

    enum BufferMapFlags
    {
        kBMF_Read = GL_MAP_READ_BIT,
        kBMF_Write = GL_MAP_WRITE_BIT,
        kBMF_Unsynchronized = GL_MAP_UNSYNCHRONIZED_BIT
    };

    enum DataType
    {
        kDT_UnsignedByte = GL_UNSIGNED_BYTE,
        kDT_Float = GL_FLOAT,
        kDT_Int = GL_INT,
        kDT_UnsignedShort = GL_UNSIGNED_SHORT
    };

    enum TextureFormat
    {
        kTF_Red = GL_RED,
        kTF_RGB = GL_RGB,
        kTF_RGBA = GL_RGBA,
        kTF_RGB8 = GL_RGB8,
        kTF_RGBA8 = GL_RGBA8,
        kTF_RGB16F = GL_RGB16F,
        kTF_RGBA16F = GL_RGBA16F
    };

    enum TextureParameter
    {
        kTP_MagFilter = GL_TEXTURE_MAG_FILTER,
        kTP_MinFilter = GL_TEXTURE_MIN_FILTER,
        kTP_WrapS = GL_TEXTURE_WRAP_S,
        kTP_WrapT = GL_TEXTURE_WRAP_T,
        kTP_WrapR = GL_TEXTURE_WRAP_R,
        kTP_MaxAnisotropy = GL_TEXTURE_MAX_ANISOTROPY,
        kTP_MaxAnisotropyEXT = GL_TEXTURE_MAX_ANISOTROPY_EXT
    };

    enum TextureFilter
    {
        kTF_Linear = GL_LINEAR,
        kTF_LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
        kTF_Nearest = GL_NEAREST
    };

    enum TextureWrapMode
    {
        kTWM_Repeat = GL_REPEAT,
        kTWM_ClampToEdge = GL_CLAMP_TO_EDGE
    };

    enum PixelStoreParameter
    {
        kPSP_PackAlignment = GL_PACK_ALIGNMENT,
        kPSP_UnpackAlignment = GL_UNPACK_ALIGNMENT,
        kPSP_UnpackRowLength = GL_UNPACK_ROW_LENGTH,
        kPSP_UnpackSkipRows = GL_UNPACK_SKIP_ROWS,
        kPSP_UnpackSkipPixels = GL_UNPACK_SKIP_PIXELS
    };

    enum Capability
    {
        kC_Blend = GL_BLEND,
        kC_DepthTest = GL_DEPTH_TEST,
        kC_CullFace = GL_CULL_FACE,
        kC_Multisample = GL_MULTISAMPLE,
        kC_PrimitiveRestart = GL_PRIMITIVE_RESTART
    };

    enum DrawMode
    {
        kDM_Triangles = GL_TRIANGLES,
        kDM_TriangleStrip = GL_TRIANGLE_STRIP,
        kDM_TriangleFan = GL_TRIANGLE_FAN,
        kDM_Lines = GL_LINES,
        kDM_LineStrip = GL_LINE_STRIP,
        kDM_LineLoop = GL_LINE_LOOP
    };

    enum ClearFlags
    {
        kCF_ColorBuffer = GL_COLOR_BUFFER_BIT,
        kCF_DepthBuffer = GL_DEPTH_BUFFER_BIT
    };

    enum BlendMode
    {
        kBM_Zero = GL_ZERO,
        kBM_One = GL_ONE,
        kBM_SrcAlpha = GL_SRC_ALPHA,
        kBM_DstAlpha = GL_DST_ALPHA,
        kBM_OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
        kBM_OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA
    };

    enum ShaderType
    {
        kST_VertexShader = GL_VERTEX_SHADER,
        kST_FragmentShader = GL_FRAGMENT_SHADER,
        kST_GeometryShader = GL_GEOMETRY_SHADER
    };

    enum ShaderProperty
    {
        kSP_CompileStatus = GL_COMPILE_STATUS,
        kSP_InfoLogLength = GL_INFO_LOG_LENGTH
    };

    enum ProgramProperty
    {
        kPP_LinkStatus = GL_LINK_STATUS,
        kPP_InfoLogLength = GL_INFO_LOG_LENGTH
    };

    enum FramebufferAttachment
    {
        kFBA_ColorAttachment0 = GL_COLOR_ATTACHMENT0,
        kFBA_DepthAttachment = GL_DEPTH_ATTACHMENT,
        kFBA_DepthStencilAttachment = GL_DEPTH_STENCIL_ATTACHMENT
    };

    enum FramebufferStatus
    {
        kFBS_Undefined = GL_FRAMEBUFFER_UNDEFINED,
        kFBS_IncompleteAttachment = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
        kFBS_IncompleteMissingAttachment = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
        kFBS_IncompleteDrawBuffer = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
        kFBS_IncompleteReadBuffer = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
        kFBS_Unsupported = GL_FRAMEBUFFER_UNSUPPORTED,
        kFBS_IncompleteMultisample = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
        kFBS_IncompleteLayerTargets = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,
        kFBS_Complete = GL_FRAMEBUFFER_COMPLETE
    };

    enum DepthFunc
    {
        kDF_Never = GL_NEVER,
        kDF_Less = GL_LESS,
        kDF_LessOrEqual = GL_LEQUAL,
        kDF_Equal = GL_EQUAL,
        kDF_NotEqual = GL_NOTEQUAL,
        kDF_GreaterOrEqual = GL_GEQUAL,
        kDF_Greater = GL_GREATER,
        kDF_Always = GL_ALWAYS
    };

    enum RenderbufferTarget
    {
        kRT_Renderbuffer = GL_RENDERBUFFER
    };

    enum RenderbufferFormat
    {
        kRF_Depth24Stencil8 = GL_DEPTH24_STENCIL8,
        kRF_DepthComponent24 = GL_DEPTH_COMPONENT24
    };

    inline const char *getVendor()
    {
        return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    }

    inline const char *getRenderer()
    {
        return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    }

    /* Textures */
    inline void activeTexture(GLuint tex)
    {
        glActiveTexture(GL_TEXTURE0 + tex);
    }

    inline void pixelStorei(PixelStoreParameter param, GLint val)
    {
        glPixelStorei(param, val);
    }

    inline void genTextures(GLsizei cnt, GLuint *dst)
    {
        glGenTextures(cnt, dst);
    }

    inline void deleteTextures(GLsizei cnt, const GLuint *src)
    {
        glDeleteTextures(cnt, src);
    }

    inline GLuint genTexture()
    {
        GLuint ret;
        glGenTextures(1, &ret);
        return ret;
    }

    inline void deleteTexture(GLuint src)
    {
        glDeleteTextures(1, &src);
    }

    inline void bindTexture(TextureTarget target, GLuint id)
    {
        glBindTexture(target, id);
    }

    inline void texStorage2D(TextureTarget target, GLint levels, TextureFormat iFormat, GLuint width, GLuint height)
    {
        glTexStorage2D(target, levels, iFormat, width, height);
    }

    inline void texStorage3D(TextureTarget target, GLint levels, TextureFormat iFormat, GLsizei width, GLsizei height, GLsizei depth)
    {
        glTexStorage3D(target, levels, iFormat, width, height, depth);
    }

    inline void texImage2D(TextureTarget target, GLint level, TextureFormat iFormat, GLuint width,
                           GLuint height, GLint border, TextureFormat format, DataType type, const void *pixels)
    {
        glTexImage2D(target, level, iFormat, width, height, border, format, type, pixels);
    }

    inline void texImage3D(TextureTarget target, GLint level, TextureFormat iFormat, GLuint width,
                           GLuint height, GLuint depth, GLuint border, TextureFormat format, DataType type, const void *pixels)
    {
        glTexImage3D(target, level, iFormat, width, height, depth, border, format, type, pixels);
    }

    inline void texSubImage2D(TextureTarget target, GLint level, GLint x, GLint y, GLuint width,
                              GLuint height, TextureFormat format, DataType type, const void *pixels)
    {
        glTexSubImage2D(target, level, x, y, width, height, format, type, pixels);
    }

    inline void texSubImage3D(TextureTarget target, GLint level, GLint xOffset, GLint yOffset, GLint zOffset, GLsizei width,
            GLsizei height, GLsizei depth, TextureFormat format, DataType type, const void *pixels)
    {
        glTexSubImage3D(target, level, xOffset, yOffset, zOffset, width, height, depth, format, type, pixels);
    }

    inline void getTexImage(TextureTarget target, GLint level, TextureFormat format, DataType type, void *pixels)
    {
        glGetTexImage(target, level, format, type, pixels);
    }

    inline void texParameteri(TextureTarget target, TextureParameter param, GLint val)
    {
        glTexParameteri(target, param, val);
    }

    inline void texParameterf(TextureTarget target, TextureParameter param, GLfloat val)
    {
        glTexParameterf(target, param, val);
    }

    /* Vertex arrays */
    inline void genVertexArrays(GLsizei cnt, GLuint *dst)
    {
        glGenVertexArrays(cnt, dst);
    }

    inline GLuint genVertexArray()
    {
        GLuint ret;
        glGenVertexArrays(1, &ret);
        return ret;
    }

    inline void deleteVertexArrays(GLsizei cnt, const GLuint *dst)
    {
        glDeleteVertexArrays(cnt, dst);
    }

    inline void deleteVertexArray(GLuint dst)
    {
        glDeleteVertexArrays(1, &dst);
    }

    inline void bindVertexArray(GLuint id)
    {
        glBindVertexArray(id);
    }

    inline void vertexAttribPointer(GLuint index, GLint size, DataType type, bool normalized, GLsizei stride, const void *pointer)
    {
        glVertexAttribPointer(index, size, type, static_cast<GLboolean>(normalized), stride, pointer);
    }

    inline void vertexAttribIPointer(GLuint index, GLint size, DataType type, GLsizei stride, const void *pointer)
    {
        glVertexAttribIPointer(index, size, type, stride, pointer);
    }

    inline void enableVertexAttribArray(GLuint index)
    {
        glEnableVertexAttribArray(index);
    }

    /* Vertex buffers */
    inline void genBuffers(GLsizei cnt, GLuint *dst)
    {
        glGenBuffers(cnt, dst);
    }

    inline GLuint genBuffer()
    {
        GLuint ret;
        glGenBuffers(1, &ret);
        return ret;
    }

    inline void deleteBuffers(GLsizei cnt, const GLuint *src)
    {
        glDeleteBuffers(cnt, src);
    }

    inline void deleteBuffer(GLuint id)
    {
        glDeleteBuffers(1, &id);
    }

    inline void bindBuffer(BufferTarget target, GLuint id)
    {
        glBindBuffer(target, id);
    }

    inline void bufferData(BufferTarget target, GLsizeiptr size, const void *data, BufferUsage usage)
    {
        glBufferData(target, size, data, usage);
    }

    inline void bufferSubData(BufferTarget target, GLintptr offset, GLsizeiptr size, const void *data)
    {
        glBufferSubData(target, offset, size, data);
    }

    inline void *mapBuffer(BufferTarget target, BufferAccess access)
    {
        return glMapBuffer(target, access);
    }

    inline void *mapBufferRange(BufferTarget target, GLintptr offset, GLsizeiptr size, GLbitfield flags)
    {
        return glMapBufferRange(target, offset, size, flags);
    }

    inline void unmapBuffer(BufferTarget target)
    {
        glUnmapBuffer(target);
    }

    /* Shaders */
    inline GLuint createShader(ShaderType st)
    {
        return glCreateShader(st);
    }

    inline void deleteShader(GLuint id)
    {
        glDeleteShader(id);
    }

    inline void shaderSource(GLuint id, GLsizei cnt, const GLchar *const *string, const GLint *len)
    {
        glShaderSource(id, cnt, string, len);
    }

    inline void compileShader(GLuint id)
    {
        glCompileShader(id);
    }

    inline void getShaderiv(GLuint id, ShaderProperty prop, GLint *dst)
    {
        glGetShaderiv(id, prop, dst);
    }

    inline void getShaderInfoLog(GLuint id, GLsizei bufSize, GLsizei *len, GLchar *log)
    {
        glGetShaderInfoLog(id, bufSize, len, log);
    }

    /* Programs */
    inline GLuint createProgram()
    {
        return glCreateProgram();
    }

    inline void deleteProgram(GLuint id)
    {
        glDeleteProgram(id);
    }

    inline void attachShader(GLuint prog, GLuint shader)
    {
        glAttachShader(prog, shader);
    }

    inline void linkProgram(GLuint prog)
    {
        glLinkProgram(prog);
    }

    inline void getProgramiv(GLuint prog, ProgramProperty prop, GLint *dst)
    {
        glGetProgramiv(prog, prop, dst);
    }

    inline void getProgramInfoLog(GLuint id, GLsizei bufSize, GLsizei *len, GLchar *log)
    {
        glGetProgramInfoLog(id, bufSize, len, log);
    }

    inline GLint getUniformLocation(GLuint prog, const GLchar *name)
    {
        return glGetUniformLocation(prog, name);
    }

    inline void uniformMatrix4fv(GLint loc, GLsizei cnt, bool transpose, const GLfloat *src)
    {
        glUniformMatrix4fv(loc, cnt, static_cast<GLboolean>(transpose), src);
    }

    inline void uniform1i(GLint loc, GLint value)
    {
        glUniform1i(loc, value);
    }

    inline void uniform4f(GLint loc, GLfloat a, GLfloat b, GLfloat c, GLfloat d)
    {
        glUniform4f(loc, a, b, c, d);
    }

    inline void uniform4fv(GLint loc, GLsizei cnt, const GLfloat *src)
    {
        glUniform4fv(loc, cnt, src);
    }

    inline void uniform2f(GLint loc, GLfloat a, GLfloat b)
    {
        glUniform2f(loc, a, b);
    }

    inline void uniform3f(GLint loc, GLfloat a, GLfloat b, GLfloat c)
    {
        glUniform3f(loc, a, b, c);
    }

    inline void uniform3fv(GLint loc, GLsizei cnt, const GLfloat *src)
    {
        glUniform3fv(loc, cnt, src);
    }

    inline void bindAttribLocation(GLuint prog, GLuint loc, const GLchar *name)
    {
        glBindAttribLocation(prog, loc, name);
    }

    /* Framebuffers */
    inline void genFramebuffers(GLsizei cnt, GLuint *dst)
    {
        glGenFramebuffers(cnt, dst);
    }

    inline GLuint genFramebuffer()
    {
        GLuint ret;
        glGenFramebuffers(1, &ret);
        return ret;
    }

    inline void deleteFramebuffers(GLsizei cnt, const GLuint *src)
    {
        glDeleteFramebuffers(cnt, src);
    }

    inline void deleteFramebuffer(GLuint id)
    {
        glDeleteFramebuffers(1, &id);
    }

    inline void bindFramebuffer(FramebufferTarget ft, GLuint id)
    {
        glBindFramebuffer(ft, id);
    }

    inline void framebufferTexture2D(FramebufferTarget ft, FramebufferAttachment at, TextureTarget tt, GLuint texId, GLint level)
    {
        glFramebufferTexture2D(ft, at, tt, texId, level);
    }

    inline void framebufferRenderbuffer(FramebufferTarget ft, FramebufferAttachment at, RenderbufferTarget rt, GLuint rb)
    {
        glFramebufferRenderbuffer(ft, at, rt, rb);
    }

    inline FramebufferStatus checkFramebufferStatus(FramebufferTarget ft)
    {
        return static_cast<FramebufferStatus>(glCheckFramebufferStatus(ft));
    }

    /* Misc */
    inline void enable(Capability cap)
    {
        glEnable(cap);
    }

    inline void disable(Capability cap)
    {
        glDisable(cap);
    }

    inline void clearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        glClearColor(r, g, b, a);
    }

    inline void clearDepth(GLfloat d)
    {
        glClearDepth(d);
    }

    inline void clear(GLbitfield what)
    {
        glClear(what);
    }

    inline void blendFunc(BlendMode sfactor, BlendMode dfactor)
    {
        glBlendFunc(sfactor, dfactor);
    }

    inline void colorMask(bool r, bool g, bool b, bool a)
    {
        glColorMask(static_cast<GLboolean>(r), static_cast<GLboolean>(g), static_cast<GLboolean>(b), static_cast<GLboolean>(a));
    }

    inline void depthMask(bool dm)
    {
        glDepthMask(static_cast<GLboolean>(dm));
    }

    inline void depthFunc(DepthFunc func)
    {
        glDepthFunc(static_cast<GLenum>(func));
    }

    inline DepthFunc getDepthFunc()
    {
        int ret;
        glGetIntegerv(GL_DEPTH_FUNC, &ret);
        return static_cast<DepthFunc>(ret);
    }

    inline void viewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        glViewport(x, y, width, height);
    }

    inline void drawArrays(DrawMode mode, GLint first, GLsizei count)
    {
        glDrawArrays(mode, first, count);
    }

    inline void drawElements(DrawMode mode, GLsizei count, DataType type, const void *indices)
    {
        glDrawElements(mode, count, type, indices);
    }

    inline int getInteger(GLenum pname)
    {
        int ret;
        glGetIntegerv(pname, &ret);
        return ret;
    }

    inline void primitiveRestartIndex(GLuint idx)
    {
        glPrimitiveRestartIndex(idx);
    }

    inline void generateMipmap(TextureTarget tt)
    {
        glGenerateMipmap(tt);
    }

    inline void useProgram(GLuint prog)
    {
        glUseProgram(prog);
    }

    inline GLuint genRenderbuffer()
    {
        GLuint ret;
        glGenRenderbuffers(1, &ret);

        return ret;
    }

    inline void genRenderbuffers(GLsizei count, GLuint *ptr)
    {
        glGenRenderbuffers(count, ptr);
    }

    inline void deleteRenderbuffer(GLuint rb)
    {
        glDeleteRenderbuffers(1, &rb);
    }

    inline void deleteRenderbuffers(GLsizei count, GLuint *ptr)
    {
        glDeleteRenderbuffers(count, ptr);
    }

    inline void bindRenderbufffer(RenderbufferTarget target, GLuint id)
    {
        glBindRenderbuffer(target, id);
    }

    inline void renderbufferStorage(RenderbufferTarget target, RenderbufferFormat iFormat, GLsizei w, GLsizei h)
    {
        glRenderbufferStorage(target, iFormat, w, h);
    }

};
