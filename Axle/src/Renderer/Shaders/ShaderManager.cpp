#include "axpch.hpp"

#include "ShaderManager.hpp"
#include "Core/Error/Error.hpp"
#include "Core/Error/Result.hpp"
#include "Core/Logger/Log.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Shaders/Shader.hpp"

namespace Axle {
    std::unique_ptr<ShaderManager> ShaderManager::s_Instance = nullptr;

    void ShaderManager::Init() {
        s_Instance = std::make_unique<ShaderManager>();
        AX_CORE_INFO(LogChannel::Renderer, "ShaderManager has been created");
    }

    void ShaderManager::Shutdown() {
        s_Instance.reset();
        AX_CORE_INFO(LogChannel::Renderer, "ShaderManager has been deleted");
    }

    Result<Ref<Shader>> ShaderManager::GetImpl(const std::string& name) {
        auto foundW = m_ShadersWeak.find(name);
        auto foundS = m_ShadersStrong.find(name);

        if (foundS != m_ShadersStrong.end())
            return foundS->second;

        if (foundW != m_ShadersWeak.end()) {
            if (foundW->second.Expired()) {
                m_ShadersWeak.erase(foundW);
                return Result<Ref<Shader>>::Err(
                    Error(ErrorCode::NotFound, "Shader with name: " + name + " is not cached"));
            } else
                return foundW->second.Lock();
        }

        return Result<Ref<Shader>>::Err(Error(ErrorCode::NotFound, "Shader with name " + name + " is not cached"));
    }

    void ShaderManager::AddImpl(const std::string& name, const Ref<Shader>& shader, bool onlyCache) {
        if (onlyCache)
            m_ShadersWeak[name] = WeakRef<Shader>(shader);
        else
            m_ShadersStrong[name] = shader;
    }

    Ref<Shader> ShaderManager::LoadImpl(const std::string& name, const std::string& path, bool onlyCache) {
        // The create method already caches the loaded shader
        Ref<Shader> ref = Shader::Create(path, name);

        // Only add it to the strong refence map if specified
        if (!onlyCache)
            AddImpl(name, ref, false);

        return ref;
    }
} // namespace Axle
