#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Types.hpp"

namespace Axle {
    template <typename T>
    concept ConfigIntegerType =
        std::same_as<T, i32> || std::same_as<T, u32> || std::same_as<T, i64> || std::same_as<T, u64> ||
        std::same_as<T, i16> || std::same_as<T, u16> || std::same_as<T, i8> || std::same_as<T, u8>;

    template <typename T>
    concept ConfigDecimalType = std::same_as<T, f32> || std::same_as<T, f64>;

    template <typename T>
    concept ConfigStringType = std::same_as<T, std::string>;

    template <typename T>
    concept ConfigType = ConfigIntegerType<T> || ConfigDecimalType<T> || ConfigStringType<T> || std::same_as<T, bool>;

    class AXLE_API Config {
    public:
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        Config();
        ~Config();

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

        /**
         * Saves the changes made to the file on disk.
         *
         * Thread safe
         * */
        inline static void Save() {
            s_Instance->SaveImpl();
        }

        /**
         * Gets a value from the config
         *
         * @param section The name of the section the key is in
         * @param name The name of the key
         *
         * @returns An Expected which contains the value if it does exist
         *
         * Thread safe
         * */
        template <ConfigType T>
        inline static Expected<T> Get(const std::string& section, const std::string& name) {
            std::scoped_lock lock(s_Instance->m_Mutex);
            return s_Instance->GetUnsafe<T>(section, name);
        }

        /**
         * Sets the given value and returns if there has been any errors.
         *
         * If the key/name or section doesn't exist it will be automatically added.
         *
         * @param section The name of the section the key is in
         * @param name The name of the key
         * @param value The new value to set
         *
         * @returns true if the operation was successful, false otherwise
         *
         * Thread safe
         * */
        template <ConfigType T>
        inline static bool Set(const std::string& section, const std::string& name, const T& value) {
            std::scoped_lock lock(s_Instance->m_Mutex);
            return s_Instance->SetUnsafe<T>(section, name, value);
        }

        /**
         * Atomically gets a value if it exists, or sets and returns a default if it doesn't.
         * Unlike calling Get() then Set() separately, this holds the lock for the whole
         * operation so no other thread can race between the check and the write.
         *
         * @param section The name of the section the key is in
         * @param name The name of the key
         * @param defaultValue The value to set and return if the key doesn't exist
         *
         * @returns The existing value, or defaultValue if it was just created
         *
         * Thread safe
         * */
        template <ConfigType T>
        static T GetOrSet(const std::string& section, const std::string& name, const T& defaultValue);

        /**
         * Deletes a key/name from the given section.
         *
         * @param section The section where the name is located
         * @param name The key/name to delete
         *
         * @returns true if the operation was successful, false otherwise
         *
         * Thread safe
         * */
        inline static bool DeleteName(const std::string& section, const std::string& name) {
            return s_Instance->DeleteImpl(section.c_str(), name.c_str());
        }

        /**
         * Deletes an entire section
         *
         * @param section The section to delete
         *
         * @returns true if the operation was successful, false otherwise
         *
         * Thread safe
         * */
        inline static bool DeleteSection(const std::string& section) {
            return s_Instance->DeleteImpl(section.c_str(), nullptr);
        }

    private:
        struct Impl; // defined in Config.cpp — keeps SimpleIni.h out of this header

        // Implementations of the static methods
        void SaveImpl();
        bool DeleteImpl(const char* section, const char* name);

        // Unsafe functions
        bool DoesKeyExistUnsafe(const std::string& section, const std::string& name);
        template <ConfigType T>
        Expected<T> GetUnsafe(const std::string& section, const std::string& name);
        template <ConfigType T>
        bool SetUnsafe(const std::string& section, const std::string& name, const T& value);

        // Specialized getters and setters
        bool GetBoolValueUnsafe(const std::string& section, const std::string& name);
        i64 GetIntegerValueUnsafe(const std::string& section, const std::string& name);
        f64 GetDecimalValueUnsafe(const std::string& section, const std::string& name);
        const char* GetStringValueUnsafe(const std::string& section, const std::string& name);

        bool SetBoolValueUnsafe(const std::string& section, const std::string& name, bool val);
        bool SetIntegerValueUnsafe(const std::string& section, const std::string& name, i64 val);
        bool SetDecimalValueUnsafe(const std::string& section, const std::string& name, f64 val);
        bool SetStringValueUnsafe(const std::string& section, const std::string& name, const char* val);

        static std::unique_ptr<Config> s_Instance;

        std::unique_ptr<Impl> m_Impl;
        std::string m_File;

        std::mutex m_Mutex;
    };


    template <ConfigType T>
    Expected<T> Config::GetUnsafe(const std::string& section, const std::string& name) {
        if (!DoesKeyExistUnsafe(section, name))
            return Expected<T>::FromException(std::invalid_argument("Can't access a non-existing element"));

        if constexpr (std::same_as<T, bool>) {
            return GetBoolValueUnsafe(section, name);
        } else if constexpr (ConfigIntegerType<T>) {
            return static_cast<T>(GetIntegerValueUnsafe(section, name));
        } else if constexpr (ConfigDecimalType<T>) {
            return static_cast<T>(GetDecimalValueUnsafe(section, name));
        } else if constexpr (ConfigStringType<T>) {
            return std::string(GetStringValueUnsafe(section, name));
        }
    }

    template <ConfigType T>
    bool Config::SetUnsafe(const std::string& section, const std::string& name, const T& value) {
        if constexpr (std::same_as<T, bool>) {
            return SetBoolValueUnsafe(section, name, value);
        } else if constexpr (ConfigIntegerType<T>) {
            return SetIntegerValueUnsafe(section, name, static_cast<i64>(value));
        } else if constexpr (ConfigDecimalType<T>) {
            return SetDecimalValueUnsafe(section, name, static_cast<f64>(value));
        } else if constexpr (ConfigStringType<T>) {
            return SetStringValueUnsafe(section, name, value.c_str());
        }
    }


    template <ConfigType T>
    T Config::GetOrSet(const std::string& section, const std::string& name, const T& defaultValue) {
        std::scoped_lock lock(s_Instance->m_Mutex);

        Expected<T> exp = s_Instance->GetUnsafe<T>(section, name);

        // Key found
        if (exp.IsValid())
            return exp.Unwrap();

        // Key missing — write the default under the same lock
        if (!s_Instance->SetUnsafe<T>(section, name, defaultValue))
            AX_CORE_ERROR(LogChannel::Config, "Error setting a value");

        return defaultValue;
    }
} // namespace Axle
