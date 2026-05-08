#include "axpch.hpp"

#include "ResourceManager.hpp"
#include "../Types.hpp"
#include "../Logger/Log.hpp"

#include <mio/mmap.hpp>
#include <system_error>

namespace Axle {
    std::unique_ptr<ResourceManager> ResourceManager::s_ResourceManager;

    void ResourceManager::Init() {
        if (s_ResourceManager != nullptr) {
            AX_CORE_WARN("Init method of the Resource Manager has been called a second time. IGNORING");
            return;
        }

        s_ResourceManager = std::make_unique<ResourceManager>();

        AX_CORE_INFO("Resource Manager initialized...");
    }

    void ResourceManager::ShutDown() {
        s_ResourceManager.reset();
        AX_CORE_INFO("Resource Manager deleted...");
    }


    void ResourceManager::LoadFile(std::string path) {
        std::error_code error;
        mio::mmap_source mmap = mio::make_mmap_source(path, error);
    }
} // namespace Axle
