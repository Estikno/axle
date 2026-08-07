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
     * All the functionality of this class is NOT THREAD SAFE and must only be accessed by the render thread.
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

        /**
         * Adds a shader for future use
         *
         * @param shader A reference to the shader to add
         * @param onlyCache Set to true if the purpose is only going to be to have the shader cached to not
         * recompile/relink again. This functionality also works if it's set to false but it's a bit more expensive
         * because of other factors
         * */
        inline static void Add(const Ref<Shader>& shader, bool onlyCache) {
            return s_Instance->AddImpl(shader->GetName(), shader, onlyCache);
        }

        /**
         * Adds a shader for future use
         *
         * @param name Custom name for the added shader
         * @param shader A reference to the shader to add
         * @param onlyCache Set to true if the purpose is only going to be to have the shader cached to not
         * recompile/relink again. This functionality also works if it's set to false but it's a bit more expensive
         * because of other factors
         * */
        inline static void Add(const std::string& name, const Ref<Shader>& shader, bool onlyCache) {
            return s_Instance->AddImpl(name, shader, onlyCache);
        }

        /**
         * Loads a shader and automatically addes it for future use
         *
         * @param path The path of the shader file
         * @param onlyCache Set to true if the purpose is only going to be to have the shader cached to not
         * recompile/relink again. This functionality also works if it's set to false but it's a bit more expensive
         * because of other factors
         * */
        inline static Ref<Shader> Load(const std::string& path, bool onlyCache) {
            return s_Instance->LoadImpl(path, path, onlyCache);
        }

        /**
         * Loads a shader and automatically addes it for future use
         *
         * @param name Custom name for the loaded shader
         * @param path The path of the shader file
         * @param onlyCache Set to true if the purpose is only going to be to have the shader cached to not
         * recompile/relink again. This functionality also works if it's set to false but it's a bit more expensive
         * because of other factors
         * */
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
