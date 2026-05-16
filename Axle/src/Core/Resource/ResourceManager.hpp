#pragma once

#include "axpch.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

#include <mio/mmap.hpp>

namespace Axle {
    class AXLE_TEST_API ResourceManager {
    public:
        // Helper classes
        // --------------

        /**
         * Read-only guard — multiple threads can hold this simultaneously
         *
         * Because it grants thread safety to the underlying data this class should be destroyed as soon as possible to
         * release the internal mutex.
         * */
        class ReadGuard {
        public:
            /**
             * Gets a const pointer to the data.
             *
             * When this class gets destroyed the underlying mutex is released. Until then it's safe to use the pointer.
             * However, using the pointer after destroying the class is undefined behavior.
             *
             * @returns A const pointer to the data
             * */
            const char* Data() const {
                return m_Ptr;
            }

            /**
             * Returns the size of the retrieved data
             *
             * @returns The mentioned size
             * */
            u64 Size() const {
                return m_Size;
            }

            // Non-copyable, moveable
            ReadGuard(const ReadGuard&) = delete;
            ReadGuard& operator=(const ReadGuard&) = delete;
            ReadGuard(ReadGuard&&) = default;

        private:
            friend class ResourceManager;

            ReadGuard(const char* ptr, u64 size, std::shared_mutex& mutex)
                : m_Ptr(ptr),
                  m_Size(size),
                  m_Lock(mutex) {}

            const char* m_Ptr;
            u64 m_Size;
            std::shared_lock<std::shared_mutex> m_Lock;
        };

        /**
         * Read-write guard — exclusive access
         *
         * Because it grants thread safety to the underlying data this class should be destroyed as soon as possible to
         * release the internal mutex.
         * */
        class WriteGuard {
        public:
            /**
             * Gets a pointer to the data.
             *
             * When this class gets destroyed the underlying mutex is released. Until then it's safe to use the pointer.
             * However, using the pointer after destroying the class is undefined behavior.
             *
             * @returns A pointer to the data
             * */
            char* Data() {
                return m_Ptr;
            }

            /**
             * Returns the size of the retrieved data
             *
             * @returns The mentioned size
             * */
            u64 Size() const {
                return m_Size;
            }

            // Non-copyable, moveable
            WriteGuard(const WriteGuard&) = delete;
            WriteGuard& operator=(const WriteGuard&) = delete;
            WriteGuard(WriteGuard&&) = default;

        private:
            friend class ResourceManager;

            WriteGuard(char* ptr, u64 size, std::shared_mutex& mutex)
                : m_Ptr(ptr),
                  m_Size(size),
                  m_Lock(mutex) {}

            char* m_Ptr;
            u64 m_Size;
            std::unique_lock<std::shared_mutex> m_Lock;
        };
        // --------------

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        ResourceManager() {}
        ~ResourceManager() {}

        /**
         * Initializes the event handler and its singleton
         * This method is NOT thread safe.
         *
         * Important: This has to be called before using the macros and any other functionality
         *
         * It is safe to call multiple times, it simply displays a warning after the first call.
         */
        static void Init();

        /**
         * Shutdowns the manager, important to call when no other component depends on it anymore
         * This method is NOT thread safe.
         */
        static void ShutDown();

        inline static ResourceManager& GetInstance() {
            return *s_ResourceManager;
        }

        /**
         * Loads the given file into memory and returns a handle to it.
         *
         * This method is thread safe.
         *
         * @param path A filesystem path to the file to be loaded
         * @param readOnly Defines if the loaded file can be modified or not
         *
         * @returns A handle to the loaded file.
         * */
        Expected<FileHandle> Load(const std::filesystem::path& path, bool readOnly = true);

        // TODO: Add reference counting. This way a resource is close if all users have released their handle
        /**
         * Closes the file associated with the given handle.
         * The given handle becomes invalid once the file is closed.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was successful, false otherwise
         * */
        bool Close(FileHandle handle);

        /**
         * Syncs current changes made to the map to disk.
         * Flushing changes of a read-only file does nothing.
         *
         * This method is thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was successful, false otherwise
         * */
        bool Sync(FileHandle handle);

        /**
         * Gets a Guard that protects thread safety to the retrieved data.
         * This method is thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns A ReadGuard used for reading/writting safely the contents
         * */
        Expected<WriteGuard> Data(FileHandle handle);

        /**
         * Gets a Guard that protects thread safety to the retrieved data.
         * behavior.
         * This method is thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns A ReadGuard used for reading safely the contents
         * */
        Expected<ReadGuard> DataConst(FileHandle handle) const;

        /**
         * Gets the size of the given resource.
         * This method is thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected type with a valid size if the handle was valid
         * */
        Expected<u64> Size(FileHandle handle) const;

        /**
         * Creates a new file with the given size in Bytes.
         * This method is thread safe if you do not create the same file twice at the same time.
         *
         * @param path The path to the new file. Obviously the file desn't have to exist.
         * @param size The size of the new file in bytes.
         *
         * @returns true if the operation was successful, false otherwise
         * */
        bool Create(const std::filesystem::path& path, u64 size);

        /**
         * Checks wether or not the given handle is valid.
         * This method is thread Safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the handle is valid, false otherwise
         * */
        bool IsHandleValid(FileHandle handle) const {
            std::shared_lock lock(m_Mutex);
            return IsHandleValidUnsafe(handle);
        }

        /**
         * Checks if the file is read-only or not.
         * This method is thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected that contains the result if valid
         * */
        Expected<bool> IsReadOnly(FileHandle handle) const;

        /**
         * Returns the path of a file.
         * This method is thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected that contains the path if valid
         * */
        Expected<std::filesystem::path> GetPath(FileHandle handle) const;

#ifdef AXLE_TESTING
        u16 LargestAvailableIndex() {
            std::shared_lock lock(m_Mutex);
            return m_LargestAvailableIndex;
        }
        std::priority_queue<u32, std::vector<u32>, std::greater<u32>> AvailableIndexes() {
            std::shared_lock lock(m_Mutex);
            return m_AvailableIndexes;
        }
        u16 MagicNumberCounter() {
            std::shared_lock lock(m_Mutex);
            return m_MagicNumberCounter;
        }
#endif // AXLE_TESTING

    private:
        /// Small struct designed to keep resources organized
        struct Resource {
            std::variant<mio::mmap_source, mio::mmap_sink> mmap;
            std::filesystem::path path;
            u32 magic;
            std::unique_ptr<std::shared_mutex> m_Mutex;
        };

        /**
         * Checks if a given file has already been opened and returns the handle if it has been.
         *
         * This method is not thread safe.
         *
         * @param path The filesystem path to the file
         *
         * @return An Expected handle value, valid if it has been already opened, otherwise not.
         * */
        Expected<FileHandle> IsAlreadyOpenedUnsafe(const std::filesystem::path& path) const;

        /**
         * Syncs current changes made to the map to disk.
         * Flushing changes of a read-only file does nothing.
         *
         * Unlike the public Sync method this one is not thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was successful, false otherwise
         * */
        bool SyncUnsafe(FileHandle handle);

        /**
         * Checks wether or not the given handle is valid.
         * Unlike the public method this one is not thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the handle is valid, false otherwise
         * */
        bool IsHandleValidUnsafe(FileHandle handle) const {
            return m_Resources.Has(GetIndexFromHandle(handle)) &&
                   m_Resources.Get(GetIndexFromHandle(handle)).Unwrap().get().magic == GetMagicFromHandle(handle);
        }

        /**
         * Checks if the given path points to an existing file.
         *
         * This method is thread safe and it doesn't lock any mutex.
         *
         * @returns true if the file exists, flase otherwise
         * */
        inline bool DoesFileExist(const std::filesystem::path& path) const {
            return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
        }

#ifdef AXLE_TESTING
    public:
#endif // AXLE_TESTING

        /**
         * Gets the index value of a Handle. Assumes the handle is correctly built (i.e. index on the 16 bottom
         * bits and magic on the 16 upper ones)
         *
         * @param h A handle
         *
         * @return The index value of the given handle
         * */
        inline u32 GetIndexFromHandle(FileHandle h) const {
            constexpr u64 indexMask = (u64(1) << 32) - 1;
            return h & indexMask;
        }

        /**
         * Gets the magic value of a Handle. Assumes the handle is correctly built (i.e. index on the 16 bottom bits
         * and magic on the 16 upper ones)
         *
         * @param h A handle
         *
         * @return The magic value of the given handle
         * */
        inline u32 GetMagicFromHandle(FileHandle h) const {
            constexpr u64 magicMask = ~((u64(1) << 32) - 1);
            return (h & magicMask) >> 32;
        }

        /**
         * Sets the magic value of the given handle to the desired one. Assumes the handle is correctly built (i.e.
         * index on the 16 bottom bits and magic on the 16 upper ones)
         *
         * @param h A reference to the handle
         * @param magic The magic value to insert
         * */
        inline void SetMagicToHandle(FileHandle& h, u32 magic) const {
            constexpr u64 indexMask = (u64(1) << 32) - 1;
            h = (h & indexMask) | (u64(magic) << 32);
        }

        /**
         * Sets the index value of the given handle to the desired one. Assumes the handle is correctly built (i.e.
         * index on the 16 bottom bits and magic on the 16 upper ones)
         *
         * @param h A reference to the handle
         * @param index The index value to insert
         * */
        inline void SetIndexToHandle(FileHandle& h, u32 index) const {
            constexpr u64 magicMask = ~((u64(1) << 32) - 1);
            h = (h & magicMask) | u64(index);
        }

        /**
         * Builds a generic handle with the given values
         *
         * @param index The desired index of the handle
         * @param magic The desired magic of the handle
         *
         * @returns The constructed handle
         * */
        inline FileHandle MakeHandle(u32 index, u32 magic) const {
            FileHandle h = u64(index);
            SetMagicToHandle(h, magic);
            return h;
        }

#ifdef AXLE_TESTING
    private:
#endif // AXLE_TESTING

        static std::unique_ptr<ResourceManager> s_ResourceManager;

        /// A counter that stores the largest available index
        u32 m_LargestAvailableIndex = 0;
        /// Contains the deleted indexes that are available once again to be used
        std::priority_queue<u32, std::vector<u32>, std::greater<u32>> m_AvailableIndexes;

        u32 m_MagicNumberCounter = 0;
        /// The id of the SparseSet are the indexes, not the whole handle
        SparseSet<Resource> m_Resources{};

        mutable std::shared_mutex m_Mutex;
    };
} // namespace Axle
