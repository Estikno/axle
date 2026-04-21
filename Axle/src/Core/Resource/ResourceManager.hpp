#pragma once

#include "axpch.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"

namespace Axle {
    struct TextFileHandle final {
        u32 handle;
    };

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
            return *m_ResourceManager;
        }

        TextFileHandle LoadText(std::string path);
        std::string GetText(const TextFileHandle& handle);

    private:
        template <typename T>
        inline u32 GetIndexFromHandle(const T h) {
            constexpr u32 indexMask = (1 << 16) - 1;
            return h.handle & indexMask;
        }

        template <typename T>
        inline u32 GetMagicFromHandle(const T h) {
            constexpr u32 magicMask = ~((1 << 16) - 1);
            return (h.handle & magicMask) >> 16;
        }


        static std::unique_ptr<ResourceManager> m_ResourceManager;

        // Just for testing
        std::vector<std::string> m_TextResource;
    };
} // namespace Axle
