#pragma once

#include "axpch.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

#include <mio/mmap.hpp>

namespace Axle {
    // TODO: Make the class thread safety
    class AXLE_API ResourceManager {
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
         *
         * @returns A handle to the loaded file.
         * */
        FileHandle LoadFile(std::string path);

        /**
         * Loads the given file into memory and returns a handle to it.
         *
         * @param paht A null terminated string that represents a path to the file to be loaded
         *
         * @returns A handle to the loaded file.
         * */
        FileHandle LoadFile(const char* path);

        /**
         * Loads the given file into memory and returns a handle to it.
         *
         * @param paht A fylesystem path to the file to be loaded
         *
         * @returns A handle to the loaded file.
         * */
        FileHandle LoadFile(std::filesystem::path path);

        /**
         * Closes the file associated with the given handle.
         * The given handle becomes invalid once the file is closed.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was succesfull, false otherwise
         * */
        void CloseFile(FileHandle handle);

        /**
         * Syncs current changes made to the map to disk.
         *
         * Flushing changes of a read-only file does nothing.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was succesfull, false otherwise
         * */
        bool SyncFile(FileHandle handle);

    private:
        /// Small struct designed to keep resources organized
        struct Resource {
            u16 magic;
            std::variant<mio::ummap_source, mio::ummap_sink> mmap;
            std::filesystem::path path;
        };

        /**
         * Checks if a given file has already been opened and returns the handle if it has been.
         *
         * @param path The filesystem path to the file
         *
         * @return An Expected handle value, valid if it has been already opened, otherwise not.
         * */
        Expected<FileHandle> IsFileAlreadyOpened(std::filesystem::path path);

        /**
         * Gets the index value of a Handle. Assumes the handle is correctly builded (i.e. index on the 16 bottom bits
         * and magic on the 16 upper ones)
         *
         * @param h A handle
         *
         * @return The index value of the given handle
         * */
        inline u16 GetIndexFromHandle(FileHandle h) {
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
        inline u16 GetMagicFromHandle(FileHandle h) {
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
        inline void SetMagicToHandle(FileHandle& h, u16 magic) {
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
        inline void SetIndexToHandle(FileHandle& h, u16 index) {
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
        inline FileHandle MakeHandle(u16 index, u16 magic) {
            FileHandle h = u32(index);
            SetMagicToHandle(h, magic);
            return h;
        }

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
