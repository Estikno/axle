#include "axpch.hpp"

#include "ResourceManager.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "../Types.hpp"
#include "../Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include <mio/mmap.hpp>

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
        // TODO: Close all loaded files
        s_ResourceManager.reset();
        AX_CORE_INFO("Resource Manager deleted...");
    }

    FileHandle ResourceManager::LoadFile(std::string path) {
        std::filesystem::path _path = path;
        return LoadFile(_path);
    }

    FileHandle ResourceManager::LoadFile(const char* path) {
        std::filesystem::path _path = path;
        return LoadFile(_path);
    }

    FileHandle ResourceManager::LoadFile(std::filesystem::path path) {
        AX_ENSURE(std::filesystem::exists(path), "Trying to load a non-existing file.");
        // File was already opened so return a valid handle to it
        Expected<FileHandle> e = IsFileAlreadyOpened(path);
        if (e.IsValid())
            return e.Unwrap();

        std::error_code error;
        mio::mmap_source mmap = mio::make_mmap_source(path.string(), error);

        AX_ENSURE(!error, "There has been an error trying to read a file: {0}", error.message());

        // File opened succesfully
        u16 magic = m_MagicNumberCounter++;
        u16 index;
        if (m_AvailableIndexes.empty())
            index = m_LargestAvailableIndex++;
        else {
            index = m_AvailableIndexes.top();
            m_AvailableIndexes.pop();
        }
        FileHandle h = MakeHandle(index, magic);
        m_Resources.Add(index, Resource{.magic = magic, .isShared = false, .mmap = std::move(mmap), .path = path});

        return h;
    }

    Expected<FileHandle> ResourceManager::IsFileAlreadyOpened(std::filesystem::path path) {
        const std::vector<size_t> AvailableFilesIdx = m_Resources.GetList();

        for (int i = 0; i < AvailableFilesIdx.size(); ++i) {
            auto got = m_Resources.Get(AvailableFilesIdx[i]);
            if (got.IsValid()) {
                // File has already been opened, return a valid handle to it
                FileHandle h = MakeHandle((u16) AvailableFilesIdx[i], got.Unwrap().get().magic);
                return h;
            }
        }

        return Expected<FileHandle>::FromException(std::runtime_error("File has not already been opened."));
    }
} // namespace Axle
