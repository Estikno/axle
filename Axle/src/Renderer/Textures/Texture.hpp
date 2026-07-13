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

    class Texture {
    public:
        Texture(i32 width, i32 height, TextureFormat internalFormat, TextureFormat dataFormat, TextureType type);
        Texture(const std::string& file, TextureType type, bool flipVertically = true);
        ~Texture() = default;

        void SetSource(const std::string& file, bool flipVertically = true);
        void SetSource(ResourceManager::ManagedFileHandle& handle, bool flipVertically = true);

        void SetWrapping(TextureWrapMode s, TextureWrapMode t);
        void SetFiltering(TextureFilteringMode min, TextureFilteringMode mag);
        void GenerateMipmaps();
        void SetBorderColor(const glm::vec4& color);

        void Bind(u32 textureUnit);

        inline u32 GetID() const {
            return m_ID;
        }

        inline TextureType GetType() const {
            return m_Type;
        }

    private:
        u32 m_ID = 0;
        i32 m_Width, m_Height, m_NrChannels;

        ResourceManager::ManagedFileHandle m_FileHandle;

        TextureFormat m_InternalFormat, m_DataFormat;
        TextureType m_Type;
    };
} // namespace Axle
