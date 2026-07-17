#pragma once

#include "axpch.hpp"

#include "Core/Error/Result.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

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

        /**
         * A RAII wrapper around a FileHandle
         * */
        class ManagedFileHandle {
        public:
            // Default constructor (invalid handle)
            ManagedFileHandle() = default;

            // Release the reference
            ~ManagedFileHandle() {
                ReleaseRef();
            }

            // Construct from raw handle (internal use by ResourceManager)
            explicit ManagedFileHandle(FileHandle handle) noexcept
                : m_Handle(handle) {}

            // Copy constructor
            ManagedFileHandle(const ManagedFileHandle& other)
                : m_Handle(other.m_Handle) {
                AddRef();
            }

            ManagedFileHandle& operator=(const ManagedFileHandle& other) {
                if (this != &other) {
                    ReleaseRef(); // Release the current handle
                    m_Handle = other.m_Handle;
                    AddRef(); // Add a reference to the new handle
                }
                return *this;
            }

            // Move Semantics: Transfer ownership without touching ref counts
            ManagedFileHandle(ManagedFileHandle&& other) noexcept
                : m_Handle(other.m_Handle) {
                other.m_Handle = INVALID_FILE_HANDLE;
            }

            ManagedFileHandle& operator=(ManagedFileHandle&& other) noexcept {
                if (this != &other) {
                    ReleaseRef();
                    m_Handle = other.m_Handle;
                    other.m_Handle = INVALID_FILE_HANDLE;
                }
                return *this;
            }

            // Equality operators
            bool operator==(const ManagedFileHandle& other) const {
                return m_Handle == other.m_Handle;
            }
            bool operator!=(const ManagedFileHandle& other) const {
                return m_Handle != other.m_Handle;
            }

            // Accessors
            FileHandle Get() const {
                return m_Handle;
            }
            bool IsValid() const {
                return m_Handle != INVALID_FILE_HANDLE;
            }

        private:
            void AddRef();
            void ReleaseRef();

            FileHandle m_Handle = INVALID_FILE_HANDLE;
        };
        // --------------

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        ResourceManager();
        ~ResourceManager();

        /**
         * Initializes the event handler and its singleton
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         *
         * Important: This has to be called before using the macros and any other functionality
         *
         * It is safe to call multiple times, it simply displays a warning after the first call.
         */
        static void Init();

        /**
         * Shutdowns the manager, important to call when no other component depends on it anymore
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         */
        static void ShutDown();

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
        inline static Result<ManagedFileHandle> Load(const std::filesystem::path& path, bool readOnly = true) {
            return s_Instance->LoadImpl(path, readOnly);
        }

        /**
         * Syncs current changes made to the map to disk. Flushing changes of a read-only file does nothing.
         *
         * This method is thread safe.
         *
         * @param handle The ManagedFileHandle associated with the file
         *
         * @returns true if the operation was successful, false otherwise
         * */
        inline static bool Sync(const ManagedFileHandle& handle) {
            return s_Instance->SyncImpl(handle);
        }

        /**
         * Syncs current changes made to the map to disk. Flushing changes of a read-only file does nothing.
         *
         * This method is thread safe. It's highly encouraged to use the Sync method that accepts a ManagedFileHandle,
         * as it's possible this will be deleted in the future.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was successful, false otherwise
         * */
        inline static bool Sync(FileHandle handle) {
            return s_Instance->SyncImpl(handle);
        }

        /**
         * Gets a Guard that protects thread safety to the retrieved data.
         * This method is thread safe.
         *
         * @param handle The ManagedFileHandle associated with the file
         *
         * @returns A ReadGuard used for reading/writting safely the contents
         * */
        inline static Result<WriteGuard> Data(const ManagedFileHandle& handle) {
            return s_Instance->DataImpl(handle);
        }

        /**
         * Gets a Guard that protects thread safety to the retrieved data.
         *
         * This method is thread safe. It's highly encouraged to use the Data method that accepts a ManagedFileHandle,
         * as it's possible this will be deleted in the future.
         *
         * @param handle The handle associated with the file
         *
         * @returns A ReadGuard used for reading/writting safely the contents
         * */
        inline static Result<WriteGuard> Data(FileHandle handle) {
            return s_Instance->DataImpl(handle);
        }

        /**
         * Gets a Guard that protects thread safety to the retrieved data.
         * behavior.
         * This method is thread safe.
         *
         * @param handle The ManagedFileHandle associated with the file
         *
         * @returns A ReadGuard used for reading safely the contents
         * */
        inline static Result<ReadGuard> DataConst(const ManagedFileHandle& handle) {
            return s_Instance->DataConstImpl(handle);
        }

        /**
         * Gets a Guard that protects thread safety to the retrieved data.
         * behavior.
         * This method is thread safe. It's highly encouraged to use the DataConst method that accepts a
         * ManagedFileHandle, as it's possible this will be deleted in the future.
         *
         * @param handle The handle associated with the file
         *
         * @returns A ReadGuard used for reading safely the contents
         * */
        inline static Result<ReadGuard> DataConst(FileHandle handle) {
            return s_Instance->DataConstImpl(handle);
        }

        /**
         * Gets the size of the given resource.
         * This method is thread safe.
         *
         * @param handle The ManagedFileHandle associated with the file
         *
         * @returns An Expected type with a valid size if the handle was valid
         * */
        inline static Result<u64> Size(const ManagedFileHandle& handle) {
            return s_Instance->SizeImpl(handle);
        }

        /**
         * Gets the size of the given resource.
         * This method is thread safe. It's highly encouraged to use the Size method that accepts a
         * ManagedFileHandle, as it's possible this will be deleted in the future.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected type with a valid size if the handle was valid
         * */
        inline static Result<u64> Size(FileHandle handle) {
            return s_Instance->SizeImpl(handle);
        }

        /**
         * Creates a new file with the given size in Bytes.
         * This method is thread safe if you do not create the same file twice at the same time.
         *
         * @param path The path to the new file. Obviously the file desn't have to exist.
         * @param size The size of the new file in bytes.
         *
         * @returns true if the operation was successful, false otherwise
         * */
        inline static bool Create(const std::filesystem::path& path, u64 size) {
            return s_Instance->CreateImpl(path, size);
        }

        /**
         * Checks wether or not the given handle is valid.
         * This method is thread Safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the handle is valid, false otherwise
         * */
        inline static bool IsHandleValid(FileHandle handle) {
            return s_Instance->IsHandleValidImpl(handle);
        }

        /**
         * Checks if the file is read-only or not.
         * This method is thread safe.
         *
         * @param handle The ManagedFileHandle associated with the file
         *
         * @returns An Expected that contains the result if valid
         * */
        inline static Result<bool> IsReadOnly(const ManagedFileHandle& handle) {
            return s_Instance->IsReadOnlyImpl(handle);
        }

        /**
         * Checks if the file is read-only or not.
         * This method is thread safe. It's highly encouraged to use the IsReadOnly method that accepts a
         * ManagedFileHandle, as it's possible this will be deleted in the future.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected that contains the result if valid
         * */
        inline static Result<bool> IsReadOnly(FileHandle handle) {
            return s_Instance->IsReadOnlyImpl(handle);
        }

        /**
         * Returns the path of a file.
         * This method is thread safe.
         *
         * @param handle The ManagedFileHandle associated with the file
         *
         * @returns An Expected that contains the path if valid
         * */
        inline static Result<std::filesystem::path> GetPath(const ManagedFileHandle& handle) {
            return s_Instance->GetPathImpl(handle);
        }

        /**
         * Returns the path of a file.
         * This method is thread safe. It's highly encouraged to use the GetPath method that accepts a
         * ManagedFileHandle, as it's possible this will be deleted in the future.
         *
         * @param handle The handle associated with the file
         *
         * @returns An Expected that contains the path if valid
         * */
        inline static Result<std::filesystem::path> GetPath(FileHandle handle) {
            return s_Instance->GetPathImpl(handle);
        }

        /**
         * Resizes a file to the new given size. If the file size was previously larger than newSize, the remainder of
         * the file is discarded. If the file was previously smaller than newSize, the file size is increased and the
         * new area appears as if zero-filled.
         *
         * This method is thread safe. It will block until all active ReadGuards and
         * WriteGuards on this resource are released before proceeding with the resize.
         *
         * WARNING: Any ReadGuard or WriteGuard obtained AFTER this method returns will
         * point to the remapped memory at the new size. However, calling this method
         * while intentionally holding a guard on the same resource from the same thread
         * will deadlock, since the resize waits for all guards to be released.
         *
         * @param handle The ManagedFileHandle associated with the file
         * @param newSize What new size do you want
         *
         * @retruns true if the operation was successful, false otherwise
         * */
        inline static bool Resize(const ManagedFileHandle& handle, u64 newSize) {
            return s_Instance->ResizeImpl(handle, newSize);
        }

        /**
         * Resizes a file to the new given size. If the file size was previously larger than newSize, the remainder of
         * the file is discarded. If the file was previously smaller than newSize, the file size is increased and the
         * new area appears as if zero-filled.
         *
         * This method is thread safe. It will block until all active ReadGuards and
         * WriteGuards on this resource are released before proceeding with the resize.
         *
         * It's highly encouraged to use the Resize method that accepts a
         * ManagedFileHandle, as it's possible this will be deleted in the future.
         *
         * WARNING: Any ReadGuard or WriteGuard obtained AFTER this method returns will
         * point to the remapped memory at the new size. However, calling this method
         * while intentionally holding a guard on the same resource from the same thread
         * will deadlock, since the resize waits for all guards to be released.
         *
         * @param handle The ManagedFileHandle associated with the file
         * @param newSize What new size do you want
         *
         * @retruns true if the operation was successful, false otherwise
         * */
        inline static bool Resize(FileHandle handle, u64 newSize) {
            return s_Instance->ResizeImpl(handle, newSize);
        }

#ifdef AXLE_TESTING
        inline static u16 LargestAvailableIndex() {
            return s_Instance->LargestAvailableIndexImpl();
        }
        inline static std::priority_queue<u32, std::vector<u32>, std::greater<u32>> AvailableIndexes() {
            return s_Instance->AvailableIndexesImpl();
        }
        inline static u16 MagicNumberCounter() {
            return s_Instance->MagicNumberCounterImpl();
        }
#endif // AXLE_TESTING

    private:
        // Let ManagedFileHandle access the AddRef and ReleaseRef methods
        friend class ManagedFileHandle;

        Result<ManagedFileHandle> LoadImpl(const std::filesystem::path& path, bool readOnly);
        bool SyncImpl(const ManagedFileHandle& handle);
        bool SyncImpl(FileHandle handle);
        Result<WriteGuard> DataImpl(const ManagedFileHandle& handle);
        Result<WriteGuard> DataImpl(FileHandle handle);
        Result<ReadGuard> DataConstImpl(const ManagedFileHandle& handle) const;
        Result<ReadGuard> DataConstImpl(FileHandle handle) const;
        Result<u64> SizeImpl(const ManagedFileHandle& handle) const;
        Result<u64> SizeImpl(FileHandle handle) const;
        bool CreateImpl(const std::filesystem::path& path, u64 size);
        bool IsHandleValidImpl(FileHandle handle) const {
            std::shared_lock lock(m_Mutex);
            return IsHandleValidUnsafe(handle);
        }
        Result<bool> IsReadOnlyImpl(const ManagedFileHandle& handle) const;
        Result<bool> IsReadOnlyImpl(FileHandle handle) const;
        Result<std::filesystem::path> GetPathImpl(const ManagedFileHandle& handle) const;
        Result<std::filesystem::path> GetPathImpl(FileHandle handle) const;
        bool ResizeImpl(const ManagedFileHandle& handle, u64 newSize);
        bool ResizeImpl(FileHandle handle, u64 newSize);
#ifdef AXLE_TESTING
        inline u16 LargestAvailableIndexImpl() {
            std::shared_lock lock(m_Mutex);
            return m_LargestAvailableIndex;
        }
        inline std::priority_queue<u32, std::vector<u32>, std::greater<u32>> AvailableIndexesImpl() {
            std::shared_lock lock(m_Mutex);
            return m_AvailableIndexes;
        }
        inline u16 MagicNumberCounterImpl() {
            std::shared_lock lock(m_Mutex);
            return m_MagicNumberCounter;
        }
#endif // AXLE_TESTING

        /// A zero-overhead wrapper to make std::atomic movable so std::swap works in SparseSet
        struct MovableAtomic {
            std::atomic<u32> count{1};

            MovableAtomic() = default;
            MovableAtomic(u32 initial)
                : count(initial) {}

            // Prevent copying
            MovableAtomic(const MovableAtomic&) = delete;
            MovableAtomic& operator=(const MovableAtomic&) = delete;

            // Custom Move Semantics
            MovableAtomic(MovableAtomic&& other) noexcept
                : count(other.count.load(std::memory_order_relaxed)) {}

            MovableAtomic& operator=(MovableAtomic&& other) noexcept {
                if (this != &other) {
                    count.store(other.count.load(std::memory_order_relaxed), std::memory_order_relaxed);
                }
                return *this;
            }
        };

        /// Small struct designed to keep resources organized
        struct Resource;

        inline static ResourceManager& GetInstance() noexcept {
            return *s_Instance;
        }

        /**
         * Incremets the reference count associated with the given handle.
         * This method is intended to be used only by ManagedFileHandle.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was successful, flase otherwise
         * */
        bool AddRef(FileHandle handle);

        /**
         * Decrements the reference count associated with the given handle. If the count reaches 0 the file is closed.
         * This method is intended to be used only by ManagedFileHandle.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was successful, flase otherwise
         * */
        bool ReleaseRef(FileHandle handle);

        /**
         * Closes the file associated with the given handle.
         * The given handle becomes invalid once the file is closed.
         * This method is not thread safe.
         *
         * @param handle The handle associated with the file
         *
         * @returns true if the operation was successful, false otherwise
         * */
        bool CloseUnsafe(FileHandle handle);

        /**
         * Checks if a given file has already been opened and returns the handle if it has been.
         *
         * This method is not thread safe.
         *
         * @param path The filesystem path to the file
         *
         * @return An Expected handle value, valid if it has been already opened, otherwise not.
         * */
        Result<FileHandle> IsAlreadyOpenedUnsafe(const std::filesystem::path& path) const;

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
        bool IsHandleValidUnsafe(FileHandle handle) const;

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
        inline static u32 GetIndexFromHandle(FileHandle h) noexcept {
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
        inline static u32 GetMagicFromHandle(FileHandle h) noexcept {
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
        inline static void SetMagicToHandle(FileHandle& h, u32 magic) noexcept {
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
        inline static void SetIndexToHandle(FileHandle& h, u32 index) noexcept {
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
        inline static FileHandle MakeHandle(u32 index, u32 magic) noexcept {
            FileHandle h = u64(index);
            SetMagicToHandle(h, magic);
            return h;
        }

#ifdef AXLE_TESTING
    private:
#endif // AXLE_TESTING

        static std::unique_ptr<ResourceManager> s_Instance;

        /// A counter that stores the largest available index
        u32 m_LargestAvailableIndex = 0;
        /// Contains the deleted indexes that are available once again to be used
        std::priority_queue<u32, std::vector<u32>, std::greater<u32>> m_AvailableIndexes;

        u32 m_MagicNumberCounter = 0;
        /// The id of the SparseSet are the indexes, not the whole handle
        std::unique_ptr<SparseSet<Resource>> m_Resources{};

        mutable std::shared_mutex m_Mutex;
    };
} // namespace Axle
