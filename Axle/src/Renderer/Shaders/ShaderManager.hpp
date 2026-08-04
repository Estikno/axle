#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Core/Error/Result.hpp"

namespace Axle {
    /**
     * This manager ensures that no shader program is compiled/linked more times than necessary by caching.
     *
     * All the functionality of this class is NOT THREAD SAFE and should only be accessed by the render thread.
     * */
    class ShaderManager {
    public:
        // Constructor and destructor do nothing as everything is constroled via Init/Shutdown
        ShaderManager() {}
        ~ShaderManager() {}

        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        /**
         * Initializes the manager
         * */
        static void Init();

        /**
         * Shutdowns the manager
         * */
        static void Shutdown();

        /**
         * Gets a shader reference by its name
         *
         * @param name Name of the shader to get
         *
         * @returns A Result containing a shader reference if the query succeeded
         * */
        inline static Result<Ref<Shader>> Get(const std::string& name) {
            return s_Instance->GetImpl(name);
        }

        inline static void Add(const Ref<Shader>& shader, bool onlyCache) {
            return s_Instance->AddImpl(shader->GetName(), shader, onlyCache);
        }

        inline static void Add(const std::string& name, const Ref<Shader>& shader, bool onlyCache) {
            return s_Instance->AddImpl(name, shader, onlyCache);
        }

        inline static Ref<Shader> Load(const std::string& path, bool onlyCache) {
            return s_Instance->LoadImpl(path, path, onlyCache);
        }

        inline static Ref<Shader> Load(const std::string& name, const std::string& path, bool onlyCache) {
            return s_Instance->LoadImpl(name, path, onlyCache);
        }

    private:
        // Static methods' implementations
        Result<Ref<Shader>> GetImpl(const std::string& name);
        void AddImpl(const std::string& name, const Ref<Shader>& shader, bool onlyCache);
        Ref<Shader> LoadImpl(const std::string& name, const std::string& path, bool onlyCache);

        static std::unique_ptr<ShaderManager> s_Instance;

        std::unordered_map<std::string, WeakRef<Shader>> m_ShadersWeak;
        std::unordered_map<std::string, Ref<Shader>> m_ShadersStrong;
    };
} // namespace Axle
