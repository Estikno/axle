#include "axpch.hpp"

#include "ResourceManager.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "../Types.hpp"
#include "../Logger/Log.hpp"

#include <fstream>
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
        // Close all remaining files
        const std::vector<size_t> AvailableFilesIdx = s_ResourceManager->m_Resources.GetList();

        for (u64 i = 0; i < AvailableFilesIdx.size(); ++i) {
            auto got = s_ResourceManager->m_Resources.Get(AvailableFilesIdx[i]);
            if (got.IsValid()) {
                Resource& resource = got.Unwrap().get();
                FileHandle h = s_ResourceManager->MakeHandle((u32) AvailableFilesIdx[i], resource.magic);

                if (std::holds_alternative<mio::mmap_source>(resource.mmap))
                    std::get<mio::mmap_source>(resource.mmap).unmap();
                else {
                    std::error_code error;
                    mio::mmap_sink& mmap = std::get<mio::mmap_sink>(resource.mmap);
                    mmap.sync(error);

                    if (error) {
                        AX_CORE_ERROR("Error flushing a file to disk: {0}", error.message());
                    }

                    mmap.unmap();
                }

                AX_CORE_TRACE("Closed file: {0}", resource.path.string());
                s_ResourceManager->m_Resources.Remove(AvailableFilesIdx[i]);
            }
        }

        s_ResourceManager.reset();
        AX_CORE_INFO("Resource Manager deleted...");
    }

    Expected<FileHandle> ResourceManager::Load(const std::filesystem::path& path, bool readOnly) {
        if (!DoesFileExist(path)) {
            AX_CORE_ERROR("Trying to load a non-existing file: {0}", path.string());
            return Expected<FileHandle>::FromException(std::invalid_argument("Trying to load a non-existing file."));
        }

        std::unique_lock lock(m_Mutex);

        // File was already opened so return a valid handle to it
        Expected<FileHandle> e = IsAlreadyOpenedUnsafe(path);
        if (e.IsValid())
            return e.Unwrap();

        if (std::filesystem::file_size(path) == 0) {
            return Expected<FileHandle>::FromException(std::runtime_error("Cannot map empty file"));
        }

        std::error_code error;
        std::variant<mio::mmap_source, mio::mmap_sink> mmap;

        if (readOnly)
            mmap = mio::make_mmap_source(path.string(), error);
        else
            mmap = mio::make_mmap_sink(path.string(), error);


        if (error) {
            AX_CORE_ERROR("There has been an error trying to read a file: {0}", error.message());
            return Expected<FileHandle>::FromException(std::runtime_error(error.message()));
        }

        // File opened succesfully
        u32 magic = m_MagicNumberCounter++;
        u32 index;
        if (m_AvailableIndexes.empty())
            index = m_LargestAvailableIndex++;
        else {
            index = m_AvailableIndexes.top();
            m_AvailableIndexes.pop();
        }
        FileHandle h = MakeHandle(index, magic);
        m_Resources.Add(index,
                        Resource{.mmap = std::move(mmap),
                                 .path = path,
                                 .magic = magic,
                                 .m_Mutex = std::make_unique<std::shared_mutex>()});

        AX_CORE_TRACE("Loaded file: {0}", path.string());

        return h;
    }

    Expected<FileHandle> ResourceManager::IsAlreadyOpenedUnsafe(const std::filesystem::path& path) const {
        const std::vector<size_t> AvailableFilesIdx = m_Resources.GetList();

        for (u64 i = 0; i < AvailableFilesIdx.size(); ++i) {
            auto got = m_Resources.Get(AvailableFilesIdx[i]);
            if (got.IsValid() && got.Unwrap().get().path == path) {
                // File has already been opened, return a valid handle to it
                FileHandle h = MakeHandle((u32) AvailableFilesIdx[i], got.Unwrap().get().magic);
                return h;
            }
        }

        return Expected<FileHandle>::FromException(std::runtime_error("File has not already been opened."));
    }

    bool ResourceManager::Close(FileHandle handle) {
        std::unique_lock lock(m_Mutex);

        if (!IsHandleValidUnsafe(handle)) {
            AX_CORE_ERROR("Trying to close a file with an invalid handle");
            return false;
        }

        Resource& resource = m_Resources.Get(GetIndexFromHandle(handle)).Unwrap().get();

        // We sync changes to disk and the close the file
        SyncUnsafe(handle);

        if (std::holds_alternative<mio::mmap_source>(resource.mmap))
            std::get<mio::mmap_source>(resource.mmap).unmap();
        else
            std::get<mio::mmap_sink>(resource.mmap).unmap();

        AX_CORE_TRACE("Closed file: {0}", resource.path.string());
        m_Resources.Remove(GetIndexFromHandle(handle));

        // The index is now free
        m_AvailableIndexes.push(GetIndexFromHandle(handle));

        return true;
    }

    bool ResourceManager::Sync(FileHandle handle) {
        std::unique_lock lock(m_Mutex);
        return SyncUnsafe(handle);
    }

    bool ResourceManager::SyncUnsafe(FileHandle handle) {
        if (!IsHandleValidUnsafe(handle)) {
            AX_CORE_ERROR("Trying to access a file with an invalid handle");
            return false;
        }

        Resource& resource = m_Resources.Get(GetIndexFromHandle(handle)).Unwrap().get();

        // If the holded map is read-only simply return
        if (std::holds_alternative<mio::mmap_source>(resource.mmap))
            return false;

        std::error_code error;
        std::get<mio::mmap_sink>(resource.mmap).sync(error);

        if (error) {
            AX_CORE_ERROR("Error flushing a file to disk: {0}", error.message());
            return false;
        }

        AX_CORE_TRACE("Flushed changes to disk from file: {0}", resource.path.string());
        return true;
    }

    Expected<ResourceManager::WriteGuard> ResourceManager::Data(FileHandle handle) {
        std::unique_lock lock(m_Mutex);

        if (!IsHandleValidUnsafe(handle)) {
            AX_CORE_ERROR("Trying to access a file with an invalid handle");
            return Expected<ResourceManager::WriteGuard>::FromException(
                std::invalid_argument("Trying to access a file with an invalid handle"));
        }

        Resource& resource = m_Resources.Get(GetIndexFromHandle(handle)).Unwrap().get();

        if (std::holds_alternative<mio::mmap_source>(resource.mmap)) {
            AX_CORE_ERROR("Trying to get a mutable pointer to a read-only resource.");
            return Expected<ResourceManager::WriteGuard>::FromException(
                std::invalid_argument("Trying to get a mutable pointer to a read-only resource"));
        }

        mio::mmap_sink& map = std::get<mio::mmap_sink>(resource.mmap);

        return WriteGuard(map.data(), map.size(), *resource.m_Mutex);
    }

    Expected<ResourceManager::ReadGuard> ResourceManager::DataConst(FileHandle handle) const {
        std::shared_lock lock(m_Mutex);

        if (!IsHandleValidUnsafe(handle)) {
            AX_CORE_ERROR("Trying to access a file with an invalid handle");
            return Expected<ResourceManager::ReadGuard>::FromException(
                std::invalid_argument("Trying to access a file with an invalid handle"));
        }

        const Resource& resource = m_Resources.Get(GetIndexFromHandle(handle)).Unwrap().get();

        if (std::holds_alternative<mio::mmap_source>(resource.mmap)) {
            const mio::mmap_source& map = std::get<mio::mmap_source>(resource.mmap);
            return ReadGuard(map.data(), map.size(), *resource.m_Mutex);
        } else {
            const mio::mmap_sink& map = std::get<mio::mmap_sink>(resource.mmap);
            return ReadGuard(map.data(), map.size(), *resource.m_Mutex);
        }
    }


    Expected<u64> ResourceManager::Size(FileHandle handle) const {
        std::shared_lock lock(m_Mutex);

        if (!IsHandleValidUnsafe(handle)) {
            AX_CORE_ERROR("Trying to access a resource with an invalid handle");
            return Expected<u64>::FromException(
                std::invalid_argument("Trying to access a resource with an invalid handle"));
        }

        const Resource& resource = m_Resources.Get(GetIndexFromHandle(handle)).Unwrap().get();

        if (std::holds_alternative<mio::mmap_source>(resource.mmap))
            return std::get<mio::mmap_source>(resource.mmap).size();
        else
            return std::get<mio::mmap_sink>(resource.mmap).size();
    }

    bool ResourceManager::Create(const std::filesystem::path& path, u64 size) {
        if (DoesFileExist(path)) {
            AX_CORE_ERROR("Trying to create a new file that already exists: {0}", path.string());
            return false;
        }

        std::ofstream file(path, std::ios::binary);

        if (!file.is_open()) {
            AX_ERROR("Failed to create file: {0}", path.string());
            return false;
        }
        file.close();

        // Allocate the desired size
        std::error_code ec;
        std::filesystem::resize_file(path, size, ec);

        if (ec) {
            AX_CORE_ERROR("Failed to resize the file: {0} with error: {1}", path.string(), ec.message());
            return false;
        }

        AX_CORE_TRACE("Created file: {0}, with size: {1}", path.string(), size);

        return true;
    }

    Expected<bool> ResourceManager::IsReadOnly(FileHandle handle) const {
        std::shared_lock lock(m_Mutex);

        if (!IsHandleValidUnsafe(handle)) {
            AX_CORE_ERROR("Trying to access a resource with an invalid handle");
            return Expected<bool>::FromException(
                std::invalid_argument("Trying to access a resource with an invalid handle"));
        }

        const Resource& resource = m_Resources.Get(GetIndexFromHandle(handle)).Unwrap().get();

        return std::holds_alternative<mio::mmap_source>(resource.mmap);
    }

    Expected<std::filesystem::path> ResourceManager::GetPath(FileHandle handle) const {
        std::shared_lock lock(m_Mutex);

        if (!IsHandleValidUnsafe(handle)) {
            AX_CORE_ERROR("Trying to access a resource with an invalid handle");
            return Expected<std::filesystem::path>::FromException(
                std::invalid_argument("Trying to access a resource with an invalid handle"));
        }

        const Resource& resource = m_Resources.Get(GetIndexFromHandle(handle)).Unwrap().get();
        return resource.path;
    }
} // namespace Axle
