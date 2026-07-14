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

        u32 CreateTexture(i32 width, i32 height, TextureFormat internalFormat);
        u32 CreateTexture(const std::string& path, TextureFormat internalFormat, bool flipVertically = true);

        void SetWrapping(u32 ID, TextureWrapMode s, TextureWrapMode t);
        void SetFiltering(u32 ID, TextureFilteringMode min, TextureFilteringMode mag);
        void GenerateMipmaps(u32 ID);

        void SetBorderColor(u32 ID, const glm::vec4& color);

        void Bind(u32 ID, u32 textureUnit);

        void Clear();

    private:
        static std::unique_ptr<TextureManager> s_Instance;

        std::unordered_map<std::string, u32> m_PathsToIDs;
        std::unordered_map<u32, std::string> m_IDToPaths;
        std::unordered_map<u32, ResourceManager::ManagedFileHandle> m_IDToHandle;
        std::unordered_set<u32> m_NoFileTextures;
    };
} // namespace Axle
