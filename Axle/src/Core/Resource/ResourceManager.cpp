#include "axpch.hpp"

#include "ResourceManager.hpp"
#include "../Types.hpp"
#include "../Logger/Log.hpp"

#include <fstream>
#include <sstream>

namespace Axle {
    std::unique_ptr<ResourceManager> ResourceManager::m_ResourceManager;

    void ResourceManager::Init() {
        if (m_ResourceManager != nullptr) {
            AX_CORE_WARN("Init method of the Resource Manager has been called a second time. IGNORING");
            return;
        }

        m_ResourceManager = std::make_unique<ResourceManager>();

        AX_CORE_INFO("Resource Manager initialized...");
    }

    void ResourceManager::ShutDown() {
        m_ResourceManager.reset();
        AX_CORE_INFO("Resource Manager deleted...");
    }

    TextFileHandle ResourceManager::LoadText(std::string path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            AX_CORE_ERROR("Failed to open file {0}", path);
        }
        std::stringstream ss;
        ss << file.rdbuf();
        AX_CORE_INFO(ss.str());

        return {0};
    }
} // namespace Axle
