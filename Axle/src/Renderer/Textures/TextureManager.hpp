#pragma once

#include "axpch.hpp"

#include "Core/Resource/ResourceManager.hpp"
#include "Texture.hpp"

#include "glm/fwd.hpp"

namespace Axle {
    class TextureManager {
    public:
        TextureManager() {}
        ~TextureManager() {}

        static void Init();
        static void Shutdown();

        inline static TextureManager& GetInstance() {
            return *s_Instance;
        }

        u32 CreateTexture(i32 width, i32 height, TextureFormat internalFormat, TextureType type);
        u32 CreateTexture(const std::string& path,
                          TextureFormat internalFormat,
                          TextureFormat dataFormat,
                          TextureType type,
                          bool flipVertically = true);

        void SetWrapping(u32 ID, TextureWrapMode s, TextureWrapMode t);
        void SetFiltering(u32 ID, TextureFilteringMode min, TextureFilteringMode man);
        void GenerateMipmaps();

        void SetBorderColor(const glm::vec4& color);

        void Bind(u32 textureUnit);

        void Clear();

    private:
        static std::unique_ptr<TextureManager> s_Instance;

        std::unordered_map<std::string, u32> m_PathsToIDs;
        std::unordered_map<u32, std::string> m_IDToPaths;
        std::unordered_map<u32, ResourceManager::ManagedFileHandle> m_IDToHandle;
    };
} // namespace Axle
