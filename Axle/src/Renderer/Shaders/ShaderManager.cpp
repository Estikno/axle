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

    Result<Ref<Shader>> ShaderManager::IsCached(const std::string& filename) {
        auto found = s_Instance->m_Shaders.find(filename);

        if (found != s_Instance->m_Shaders.end()) {
            if (found->second.Expired())
                return Result<Ref<Shader>>::Err(
                    Error(ErrorCode::NotFound, "Shader with file " + filename + " is not cached"));
            else
                return found->second.Lock();
        }

        return Result<Ref<Shader>>::Err(Error(ErrorCode::NotFound, "Shader with file " + filename + " is not cached"));
    }

    void ShaderManager::CacheShader(const std::string& filename, const Ref<Shader>& shader) {
        s_Instance->m_Shaders[filename] = WeakRef<Shader>(shader);
    }
} // namespace Axle
