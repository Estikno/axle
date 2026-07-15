#include "axpch.hpp"

#include <glad/gl.h>

#include "Texture.hpp"

namespace Axle {
    u32 TextureWrapModeToOpenGL(TextureWrapMode mode) {
        switch (mode) {
            case TextureWrapMode::Repeat:
                return GL_REPEAT;
            case TextureWrapMode::MirroredRepeat:
                return GL_MIRRORED_REPEAT;
            case TextureWrapMode::ClampToEdge:
                return GL_CLAMP_TO_EDGE;
            case TextureWrapMode::ClampToBorder:
                return GL_CLAMP_TO_BORDER;
        }
    }

    u32 TextureFilterToOpenGL(TextureFilteringMode mode) {
        switch (mode) {
            case TextureFilteringMode::Nearest:
                return GL_NEAREST;
            case TextureFilteringMode::Linear:
                return GL_LINEAR;
            case TextureFilteringMode::NearestMipmapNearest:
                return GL_NEAREST_MIPMAP_NEAREST;
            case TextureFilteringMode::LinearMipmapNearest:
                return GL_LINEAR_MIPMAP_NEAREST;
            case Axle::TextureFilteringMode::NearestMipmapLinear:
                return GL_NEAREST_MIPMAP_LINEAR;
            case TextureFilteringMode::LinearMipmapLinear:
                return GL_LINEAR_MIPMAP_LINEAR;
        }
    }

    u32 TextureFormatToOpenGL(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGB:
                return GL_RGB;
            case TextureFormat::RGBA:
                return GL_RGBA;
            case TextureFormat::RGB8:
                return GL_RGB8;
            case TextureFormat::RGBA8:
                return GL_RGBA8;
            case TextureFormat::RGB16F:
                return GL_RGB16F;
            case TextureFormat::RGBA16F:
                return GL_RGBA16F;
            case TextureFormat::RGB32F:
                return GL_RGB32F;
            case TextureFormat::RGBA32F:
                return GL_RGBA32F;
            case TextureFormat::R8:
                return GL_R8;
            case TextureFormat::RG8:
                return GL_RG8;
            case TextureFormat::SRGB8:
                return GL_SRGB8;
            case TextureFormat::SRGB8Alpha8:
                return GL_SRGB8_ALPHA8;
            case TextureFormat::Depth24Stencil8:
                return GL_DEPTH24_STENCIL8;
            case TextureFormat::Depth32F:
                return GL_DEPTH_COMPONENT32F;
        }
    }
} // namespace Axle
