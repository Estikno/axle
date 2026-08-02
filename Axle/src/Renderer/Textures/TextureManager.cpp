#include "axpch.hpp"

#include <glad/gl.h>

#include "TextureManager.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Result.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Renderer/GLDebug.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include <cstring>

namespace Axle {
    std::unique_ptr<TextureManager> TextureManager::s_Instance = nullptr;

    void TextureManager::Init() {
        s_Instance = std::make_unique<TextureManager>();
        AX_CORE_INFO(LogChannel::Renderer, "Texture manager initialized");
    }

    void TextureManager::Shutdown() {
        s_Instance.reset();
        AX_CORE_INFO(LogChannel::Renderer, "Texture manager deleted");
    }

    Result<Ref<Texture2D>> TextureManager::IsCached2D(const std::string& filename) {
        auto found = s_Instance->m_Texture2Ds.find(filename);

        if (found != s_Instance->m_Texture2Ds.end()) {
            if (found->second.Expired())
                return Result<Ref<Texture2D>>::Err(
                    Error(ErrorCode::NotFound, "Shader with file " + filename + " is not cached"));
            else
                return found->second.Lock();
        }

        return Result<Ref<Texture2D>>::Err(
            Error(ErrorCode::NotFound, "Shader with file " + filename + " is not cached"));
    }

    Result<Ref<TextureCubemap>> TextureManager::IsCachedCubemap(const std::string& filename) {
        auto found = s_Instance->m_TextureCubemaps.find(filename);

        if (found != s_Instance->m_TextureCubemaps.end()) {
            if (found->second.Expired())
                return Result<Ref<TextureCubemap>>::Err(
                    Error(ErrorCode::NotFound, "Shader with file " + filename + " is not cached"));
            else
                return found->second.Lock();
        }

        return Result<Ref<TextureCubemap>>::Err(
            Error(ErrorCode::NotFound, "Shader with file " + filename + " is not cached"));
    }

    void TextureManager::Cache2D(const std::string& filename, const Ref<Texture2D>& texture) {
        s_Instance->m_Texture2Ds[filename] = WeakRef<Texture2D>(texture);
    }

    void TextureManager::CacheCubemap(const std::string& filename, const Ref<TextureCubemap>& texture) {
        s_Instance->m_TextureCubemaps[filename] = WeakRef<TextureCubemap>(texture);
    }

} // namespace Axle
