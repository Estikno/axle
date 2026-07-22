#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"

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

    struct FaceExtract {
        u32 target;
        i32 col, row;
    };

    // Order matches GL_TEXTURE_CUBE_MAP_POSITIVE_X..NEGATIVE_Z (+X,-X,+Y,-Y,+Z,-Z)
    constexpr FaceExtract kFaces[6] = {
        {0x8515, 2, 1},
        {0x8516, 0, 1},
        {0x8517, 1, 0},
        {0x8518, 1, 2},
        {0x8519, 1, 1},
        {0x851A, 3, 1},
    };

    u32 TextureWrapModeToOpenGL(TextureWrapMode mode);
    u32 TextureFilterToOpenGL(TextureFilteringMode mode);
    u32 TextureFormatToOpenGL(TextureFormat format);

    void
    ExtractFaceFromHorizontalCross(const u8* data, i32 width, i32 nrChannels, i32 col, i32 row, i32 faceWidth, u8* dst);
} // namespace Axle
