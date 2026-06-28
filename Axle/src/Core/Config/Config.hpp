#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "Core/Types.hpp"

#include <SimpleIni.h>

namespace Axle {
    class AXLE_API Config {
    public:
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        // Constructors do nothing because the initialization/destruction is manual with Init/ShutDown
        Config()
            : m_Ini(false, false, false) {}
        ~Config() {}

        /**
         * Initializes the Config manager and its singleton
         *
         * Important: This has to be called before using the macros and any other functionality
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         */
        static void Init(const std::string& file);

        /**
         * Shutdowns the manager, important to call when no other component depends on it anymore
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         */
        static void ShutDown();

    private:
        void Save();

        bool DoesKeyExist(const std::string& section, const std::string& name);

        Expected<bool> GetBool(const std::string& section, const std::string& name);
        Expected<i32> GetI32(const std::string& section, const std::string& name);
        Expected<f32> GetFloat(const std::string& section, const std::string& name);

        static std::unique_ptr<Config> s_Instance;

        CSimpleIniA m_Ini;
        std::string m_File;

        std::mutex m_Mutex;
    };
} // namespace Axle
