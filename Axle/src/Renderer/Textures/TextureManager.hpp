#pragma once

#include "axpch.hpp"

#include "Texture.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Core/Error/Result.hpp"

namespace Axle {
    /**
     * Manages the texture caching of the renderer
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

        /**
         * Checks wether a texture2D is cached or not
         *
         * @param filename Path to the desired texture
         *
         * @returns A result that contains a reference to a cached texture if the request succeedes
         * */
        static Result<Ref<Texture2D>> IsCached2D(const std::string& filename);

        /**
         * Checks wether a texture cubemap is cached or not
         *
         * @param filename Path to the desired texture cubemap
         *
         * @returns A result that contains a reference to a cached texture if the request succeedes
         * */
        static Result<Ref<TextureCubemap>> IsCachedCubemap(const std::string& filename);

        /**
         * Caches a texture2D
         *
         * @param filename Path of the texture to cache
         * @param shader Counted reference to the texture to cache
         * */
        static void Cache2D(const std::string& filename, const Ref<Texture2D>& texture);

        /**
         * Caches a texture cubemap
         *
         * @param filename Path of the texture to cache
         * @param shader Counted reference to the texture to cache
         * */
        static void CacheCubemap(const std::string& filename, const Ref<TextureCubemap>& texture);

    private:
        static std::unique_ptr<TextureManager> s_Instance;

        std::unordered_map<std::string, WeakRef<Texture2D>> m_Texture2Ds;
        std::unordered_map<std::string, WeakRef<TextureCubemap>> m_TextureCubemaps;
    };
} // namespace Axle
