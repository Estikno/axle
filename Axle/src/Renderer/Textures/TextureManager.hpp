#pragma once

#include "axpch.hpp"

#include "Texture.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Core/Error/Result.hpp"

namespace Axle {
    /**
     * Manages the textures of the renderer
     *
     * ALL the functionality of this class is NOT THREAD SAFE and must only be called from the renderer thread. Loadings
     * are planned to be pararelized but it's currently not in effect
     * */
    class TextureManager {
    public:
        // Constructor and destructor do nothing as everything is constroled via Init/Shutdown
        TextureManager() {}
        ~TextureManager() {}

        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        /**
         * Initializes the system
         * */
        static void Init();
        /**
         * Properly deletes and deallocates all resources used
         * */
        static void Shutdown();

        static Result<Ref<Texture2D>> IsCached2D(const std::string& filename);
        static Result<Ref<TextureCubemap>> IsCachedCubemap(const std::string& filename);

        static void Cache2D(const std::string& filename, const Ref<Texture2D>& texture);
        static void CacheCubemap(const std::string& filename, const Ref<TextureCubemap>& texture);

    private:
        static std::unique_ptr<TextureManager> s_Instance;

        std::unordered_map<std::string, WeakRef<Texture2D>> m_Texture2Ds;
        std::unordered_map<std::string, WeakRef<TextureCubemap>> m_TextureCubemaps;
    };
} // namespace Axle
