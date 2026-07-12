#include "axpch.hpp"

#include <glad/gl.h>
#include "Texture.hpp"

namespace Axle {
    static u32 WrapModeToOpenGL(TextureWrapMode mode) {
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

    static u32 TextureFilterToOpenGL(TextureFilteringMode mode) {
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


} // namespace Axle
