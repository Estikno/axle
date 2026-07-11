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

    class Texture {
    public:
        explicit Texture(u32 width, u32 height);
        explicit Texture(u32 width, u32 height, const std::string& file);
        explicit Texture(u32 width, u32 height, ResourceManager::ManagedFileHandle& handle);

        ~Texture();

        void SetTextureWrapping(TextureWrapMode s, TextureWrapMode t);
        void SetTextureFiltering(TextureFilteringMode min, TextureFilteringMode mag);
        void GenerateMipmaps();

        void Bind(u32 textureUnit);

        inline u32 GetID() const {
            return m_ID;
        }

    private:
        u32 m_ID;
        u32 m_Width, m_Height;
        ResourceManager::ManagedFileHandle m_FileHandle;
    };
} // namespace Axle
