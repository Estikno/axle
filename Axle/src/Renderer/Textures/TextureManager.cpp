#include "axpch.hpp"

#include "TextureManager.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    std::unique_ptr<TextureManager> TextureManager::s_Instance = nullptr;

    void TextureManager::Init() {
        s_Instance = std::make_unique<TextureManager>();
        AX_CORE_INFO(LogChannel::Renderer, "Texture manager initialized");
    }

    void TextureManager::Shutdown() {}
} // namespace Axle
