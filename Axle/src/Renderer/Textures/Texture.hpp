#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Resource/ResourceManager.hpp"

namespace Axle {
    enum class TextureWrapMode { Repeat = 0, MirroredRepeat, ClampToEdge, ClampToBorder };
    enum class TextureFilteringMode {
        Nearest = 0,
        Linear,
        NearestMipmapNearest,
        LinearMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapLinear
    };
    enum class TextureFormat {
        RGB = 0,
        RGBA,
        RGB8,
        RGBA8,
        RGB16F,
        RGBA16F,
        RGB32F,
        RGBA32F,
        R8,
        RG8,
        SRGB8,
        SRGB8Alpha8,
        Depth24Stencil8,
        Depth32F
    };
    enum class TextureType { Diffuse = 0, Specular, Unknown };

    u32 TextureWrapModeToOpenGL(TextureWrapMode mode);
    u32 TextureFilterToOpenGL(TextureFilteringMode mode);
    u32 TextureFormatToOpenGL(TextureFormat format);
} // namespace Axle
