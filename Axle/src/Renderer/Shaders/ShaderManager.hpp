#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Core/Error/Result.hpp"

namespace Axle {
    class Shader;

    /**
     * This manager ensures that no shader program is compiled/linked more times than necessary by caching.
     *
     * All the functionality of this class is NOT THREAD SAFE and should only be accessed by the render thread.
     * */
    class ShaderManager {
    public:
        /**
         * Initializes the manager
         * */
        static void Init();

        /**
         * Shutdowns the manager
         * */
        static void Shutdown();

        /**
         * Checks wether a shader is cached or not
         *
         * @param filename Path to the desired shader
         *
         * @returns A result that contains a reference to a cached shader if the request succeedes
         * */
        static Result<Ref<Shader>> IsCached(const std::string& filename);

        /**
         * Caches a shader
         *
         * @param filename Path of the shader to cache
         * @param shader Counted reference to the shader to cache
         * */
        static void CacheShader(const std::string& filename, const Ref<Shader>& shader);

    private:
        static std::unique_ptr<ShaderManager> s_Instance;

        std::unordered_map<std::string, WeakRef<Shader>> m_Shaders;
    };
} // namespace Axle
