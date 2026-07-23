#include "axpch.hpp"

#include <glad/gl.h>

#include "Shader.hpp"

namespace Axle {
    u32 ShaderDataTypeSize(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:
                return 4;
            case ShaderDataType::Vec2:
                return 4 * 2;
            case ShaderDataType::Vec3:
                return 4 * 3;
            case ShaderDataType::Vec4:
                return 4 * 4;
            case ShaderDataType::Mat3:
                return 4 * 3 * 3;
            case ShaderDataType::Mat4:
                return 4 * 4 * 4;
            case ShaderDataType::Int:
                return 4;
            case ShaderDataType::Int2:
                return 4 * 2;
            case ShaderDataType::Int3:
                return 4 * 3;
            case ShaderDataType::Int4:
                return 4 * 4;
            case ShaderDataType::Bool:
                return 4;
        }

        AX_PANIC(LogChannel::Renderer, "Unknown ShaderDataType!");
    }

    u32 ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:
                return GL_FLOAT;
            case ShaderDataType::Vec2:
                return GL_FLOAT;
            case ShaderDataType::Vec3:
                return GL_FLOAT;
            case ShaderDataType::Vec4:
                return GL_FLOAT;
            case ShaderDataType::Mat3:
                return GL_FLOAT;
            case ShaderDataType::Mat4:
                return GL_FLOAT;
            case ShaderDataType::Int:
                return GL_INT;
            case ShaderDataType::Int2:
                return GL_INT;
            case ShaderDataType::Int3:
                return GL_INT;
            case ShaderDataType::Int4:
                return GL_INT;
            case ShaderDataType::Bool:
                return GL_BOOL;
        }

        AX_PANIC(LogChannel::Renderer, "Unknown ShaderDataType!");
    }
} // namespace Axle
