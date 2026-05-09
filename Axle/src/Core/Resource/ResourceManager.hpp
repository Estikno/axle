#pragma once

#include "axpch.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

#include <mio/mmap.hpp>

namespace Axle {
    // TODO: Make the class thread safety
    class AXLE_API ResourceManager {
    public:
        using FileHandle = u32;

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

        void LoadFile(std::string path);

    private:
        struct Resource {
            u16 magic;
            bool is_shared;
            std::variant<mio::mmap_source, mio::mmap_sink> mmap;
        };

        inline u16 GetIndexFromHandle(FileHandle h) {
            constexpr u32 indexMask = (1 << 16) - 1;
            return h & indexMask;
        }

        inline u16 GetMagicFromHandle(FileHandle h) {
            constexpr u32 magicMask = ~((1 << 16) - 1);
            return (h & magicMask) >> 16;
        }

        inline void SetMagicToHandle(FileHandle& h, u16 magic) {
            constexpr u32 indexMask = (1 << 16) - 1;
            h = (h & indexMask) | (u32(magic) << 16);
        }

        inline void SetIndexToHandle(FileHandle& h, u16 index) {
            constexpr u32 magicMask = ~((1 << 16) - 1);
            h = (h & magicMask) | u32(index);
        }

        static std::unique_ptr<ResourceManager> s_ResourceManager;

        /// A counter that stores the largest available index
        u16 m_LargestAvailableIndex = 0;
        /// Contains the deleted indexes that are available once again to be used
        std::priority_queue<u16, std::vector<u16>, std::greater<u16>> m_AvailableIndexes;

        u16 m_MagicNumberCounter = 0;
        SparseSet<Resource> m_Resources;
    };
} // namespace Axle
