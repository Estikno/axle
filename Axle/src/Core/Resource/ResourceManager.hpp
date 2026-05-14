#pragma once

#include "axpch.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

#include <mio/mmap.hpp>

namespace Axle {
    // TODO: Make the class thread safety
    class AXLE_TEST_API ResourceManager {
    public:
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        ResourceManager() {}
        ~ResourceManager() {}

        /**
         * Initializes the event handler and its singleton
         *
         * Important: This has to be called before using the macros and any other functionality
         *
         * It is safe to call multiple times, it simply displays a warning after the first call.
         */
        static void Init();

        /**
         * Shutdowns the manager, important to call when no other component depends on it anymore
         */
        static void ShutDown();

        inline static ResourceManager& GetInstance() {
            return *s_ResourceManager;
        }

        /**
         * Loads the given file into memory and returns a handle to it.
         *
         * @param paht A string that represents a path to the file to be loaded
         * @param readOnly Defines if the loaded file can be modified or not
         *
         * @returns A handle to the loaded file.
         * */
        Expected<FileHandle> Load(std::string path, bool readOnly = true);

        /**
         * Loads the given file into memory and returns a handle to it.
         *
         * @param paht A null terminated string that represents a path to the file to be loaded
         * @param readOnly Defines if the loaded file can be modified or not
         *
         * @returns A handle to the loaded file.
         * */
        Expected<FileHandle> Load(const char* path, bool readOnly = true);

        /**
         * Loads the given file into memory and returns a handle to it.
         *
         * @param paht A fylesystem path to the file to be loaded
         * @param readOnly Defines if the loaded file can be modified or not
         *
         * @returns A handle to the loaded file.
         * */
        Expected<FileHandle> Load(std::filesystem::path path, bool readOnly = true);

        /**
         * Closes the file associated with the given handle.
         * The given handle becomes invalid once the file is closed.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was succesfull, false otherwise
         * */
        bool Close(FileHandle handle);

        /**
         * Syncs current changes made to the map to disk.
         *
         * Flushing changes of a read-only file does nothing.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was succesfull, false otherwise
         * */
        bool Sync(FileHandle handle);

        /**
         * Gets a pointer to the mapped data. Do not delete the given pointer, otherwise it is undefined behavior.
         *
         * If this method is called with a read-only file then it fails.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected value that contains the pointer
         * */
        Expected<char*> Data(FileHandle handle);

        /**
         * Gets a constant pointer to the mapped data. Do not delete the given pointer, otherwise it is undefined
         * behavior.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected value that contains the pointer
         * */
        Expected<const char*> DataConst(FileHandle handle) const;

        /**
         * Gets the size of the given resource.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected type with a valid size if the handle was valid
         * */
        Expected<u64> Size(FileHandle handle) const;

        /**
         * Creates a new file with the given size in Bytes.
         *
         * @param path The path to the new file. Obviosly the file desn't have to exist.
         * @param size The size of the new file in bytes.
         *
         * @returns true if the operation was succesfull, false otherwise
         * */
        bool Create(std::string& path, u64 size);

        /**
         * Creates a new file with the given size in Bytes.
         *
         * @param path The path to the new file. Obviosly the file desn't have to exist.
         * @param size The size of the new file in bytes.
         *
         * @returns true if the operation was succesfull, false otherwise
         * */
        bool Create(const char* path, u64 size);

        /**
         * Creates a new file with the given size in Bytes.
         *
         * @param path The path to the new file. Obviosly the file desn't have to exist.
         * @param size The size of the new file in bytes.
         *
         * @returns true if the operation was succesfull, false otherwise
         * */
        bool Create(const std::filesystem::path& path, u64 size);

#ifdef AXLE_TESTING
        u16 LargestAvailableIndex() {
            return m_LargestAvailableIndex;
        }
        std::priority_queue<u16, std::vector<u16>, std::greater<u16>>& AvailableIndexes() {
            return m_AvailableIndexes;
        }
        u16 MagicNumberCounter() {
            return m_MagicNumberCounter;
        }
#endif // AXLE_TESTING

    private:
        /// Small struct designed to keep resources organized
        struct Resource {
            std::variant<mio::mmap_source, mio::mmap_sink> mmap;
            std::filesystem::path path;
            u16 magic;
        };

        /**
         * Checks if a given file has already been opened and returns the handle if it has been.
         *
         * @param path The filesystem path to the file
         *
         * @return An Expected handle value, valid if it has been already opened, otherwise not.
         * */
        Expected<FileHandle> IsAlreadyOpened(std::filesystem::path path) const;

        inline bool DoesFileExist(const std::filesystem::path& path) const {
            return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
        }

#ifdef AXLE_TESTING
    public:
#endif // AXLE_TESTING

        /**
         * Gets the index value of a Handle. Assumes the handle is correctly builded (i.e. index on the 16 bottom
         * bits and magic on the 16 upper ones)
         *
         * @param h A handle
         *
         * @return The index value of the given handle
         * */
        inline u16 GetIndexFromHandle(FileHandle h) const {
            constexpr u32 indexMask = (1 << 16) - 1;
            return h & indexMask;
        }

        /**
         * Gets the magic value of a Handle. Assumes the handle is correctly builded (i.e. index on the 16 bottom bits
         * and magic on the 16 upper ones)
         *
         * @param h A handle
         *
         * @return The magic value of the given handle
         * */
        inline u16 GetMagicFromHandle(FileHandle h) const {
            constexpr u32 magicMask = ~((1 << 16) - 1);
            return (h & magicMask) >> 16;
        }

        /**
         * Sets the magic value of the given handle to the desired one. Assumes the handle is correctly builded (i.e.
         * index on the 16 bottom bits and magic on the 16 upper ones)
         *
         * @param h A reference to the handle
         * @param magic The magic value to insert
         * */
        inline void SetMagicToHandle(FileHandle& h, u16 magic) const {
            constexpr u32 indexMask = (1 << 16) - 1;
            h = (h & indexMask) | (u32(magic) << 16);
        }

        /**
         * Sets the index value of the given handle to the desired one. Assumes the handle is correctly builded (i.e.
         * index on the 16 bottom bits and magic on the 16 upper ones)
         *
         * @param h A reference to the handle
         * @param index The index value to insert
         * */
        inline void SetIndexToHandle(FileHandle& h, u16 index) const {
            constexpr u32 magicMask = ~((1 << 16) - 1);
            h = (h & magicMask) | u32(index);
        }

        /**
         * Builds a generic handle with the given values
         *
         * @param index The desired index of the handle
         * @param magic The desired magic of the handle
         *
         * @returns The constructed handle
         * */
        inline FileHandle MakeHandle(u16 index, u16 magic) const {
            FileHandle h = u32(index);
            SetMagicToHandle(h, magic);
            return h;
        }

#ifdef AXLE_TESTING
    private:
#endif // AXLE_TESTING

        static std::unique_ptr<ResourceManager> s_ResourceManager;

        /// A counter that stores the largest available index
        u16 m_LargestAvailableIndex = 0;
        /// Contains the deleted indexes that are available once again to be used
        std::priority_queue<u16, std::vector<u16>, std::greater<u16>> m_AvailableIndexes;

        u16 m_MagicNumberCounter = 0;
        /// The id of the SparseSet are the indexes, not the whole handle
        SparseSet<Resource> m_Resources{};
    };
} // namespace Axle
