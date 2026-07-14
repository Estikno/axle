#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Resource/ResourceManager.hpp"

#include <glm/glm.hpp>

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
    enum class TextureFormat { RGB = 0, RGBA };
    enum class TextureType { Diffuse = 0, Specular, Unknown };

    u32 TextureWrapModeToOpenGL(TextureWrapMode mode);
    u32 TextureFilterToOpenGL(TextureFilteringMode mode);
    u32 TextureFormatToOpenGL(TextureFormat format);

    // struct Texture {
    //     u32 m_ID = 0;
    //     i32 m_Width, m_Height, m_NrChannels;
    //
    //     TextureFormat m_InternalFormat, m_DataFormat;
    //     TextureType m_Type;
    // };
} // namespace Axle
