#pragma once

#include "axpch.hpp"

#include "Core/Resource/ResourceManager.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Core/Types.hpp"

namespace Axle {
    class ShaderManager {
    public:
        ShaderManager() {}
        ~ShaderManager() {}

        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        static void Init();
        static void Shutdown();

        inline static ShaderManager& GetInstance() {
            return *s_Instance;
        }

        void Clear();

    private:
        static std::unique_ptr<ShaderManager> s_Instance;

        std::unordered_map<u32, Shader> m_IDToShader;
        std::unordered_map<Shader, u32> m_ShaderToID;
        std::unordered_map<u32, ResourceManager::ManagedFileHandle> m_IDToFileHandle;

        std::unordered_set<u32> m_ProgramsIDs;
    };
} // namespace Axle
