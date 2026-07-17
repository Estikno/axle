#include <glad/gl.h>

#include "GLDebug.hpp"

#include "Core/Logger/Log.hpp"

namespace Axle {
    const char* GLErrorToString(u32 err) {
        switch (err) {
            case GL_INVALID_ENUM:
                return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:
                return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION:
                return "GL_INVALID_OPERATION";
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "GL_INVALID_FRAMEBUFFER_OPERATION";
            case GL_OUT_OF_MEMORY:
                return "GL_OUT_OF_MEMORY";
            case GL_STACK_UNDERFLOW:
                return "GL_STACK_UNDERFLOW";
            case GL_STACK_OVERFLOW:
                return "GL_STACK_OVERFLOW";
            default:
                return "UNKNOWN_GL_ERROR";
        }
    }

    bool CheckGLErrors(const char* expr, const char* file, int line) {
        bool hadError = false;
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            hadError = true;
            AX_CORE_ERROR(LogChannel::Renderer,
                          "[OpenGL Error] {0} ({1}) at {2} in {3}:{4}",
                          GLErrorToString(err),
                          (u32) err,
                          expr,
                          file,
                          line);
        }
        return hadError;
    }
} // namespace Axle
