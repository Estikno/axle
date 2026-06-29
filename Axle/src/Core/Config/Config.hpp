#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include "Core/Types.hpp"

#include <SimpleIni.h>

namespace Axle {
    template <typename T>
    concept ConfigType = std::same_as<T, bool> || std::same_as<T, i32> || std::same_as<T, u32> ||
                         std::same_as<T, f32> || std::same_as<T, f64> || std::same_as<T, i64> || std::same_as<T, u64> ||
                         std::same_as<T, i16> || std::same_as<T, u16> || std::same_as<T, i8> || std::same_as<T, u8> ||
                         std::same_as<T, char> || std::same_as<T, std::string>;

    template <typename T>
    concept ConfigIntegerType =
        std::same_as<T, i32> || std::same_as<T, u32> || std::same_as<T, i64> || std::same_as<T, u64> ||
        std::same_as<T, i16> || std::same_as<T, u16> || std::same_as<T, i8> || std::same_as<T, u8>;

    template <typename T>
    concept ConfigDecimalType = std::same_as<T, f32> || std::same_as<T, f64>;

    template <typename T>
    concept ConfigStringType = std::same_as<T, std::string> || std::same_as<T, const char*>;

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

        /**
         * Saves the changes made to the file on disk.
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
         * */
        template <ConfigType T>
        static Expected<T> Get(const std::string& section, const std::string& name);

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
         * */
        template <ConfigType T>
        static bool Set(const std::string& section, const std::string& name, const T& value);

        /**
         * Deletes a key/name from the given section.
         *
         * @param section The section where the name is located
         * @param name The key/name to delete
         *
         * @returns true if the operation was successful, false otherwise
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
         * */
        inline static bool DeleteSection(const std::string& section) {
            return s_Instance->DeleteImpl(section.c_str(), nullptr);
        }

    private:
        // Implementations of the static methods
        void SaveImpl();
        inline bool DeleteImpl(const char* section, const char* name) {
            std::scoped_lock lock(s_Instance->m_Mutex);
            return m_Ini.Delete(section, name);
        }

        inline bool DoesKeyExistUnsafe(const std::string& section, const std::string& name) {
            return m_Ini.GetValue(section.c_str(), name.c_str()) != nullptr;
        }

        inline bool GetBoolValueUnsafe(const std::string& section, const std::string& name) {
            return m_Ini.GetBoolValue(section.c_str(), name.c_str());
        }
        inline i64 GetIntegerValueUnsafe(const std::string& section, const std::string& name) {
            return static_cast<i64>(m_Ini.GetLongValue(section.c_str(), name.c_str()));
        }
        inline f64 GetDecimalValueUnsafe(const std::string& section, const std::string& name) {
            return m_Ini.GetDoubleValue(section.c_str(), name.c_str());
        }
        inline const char* GetStringValueUnsafe(const std::string& section, const std::string& name) {
            return m_Ini.GetValue(section.c_str(), name.c_str());
        }

        inline bool SetBoolValueUnsafe(const std::string& section, const std::string& name, bool val) {
            return m_Ini.SetBoolValue(section.c_str(), name.c_str(), val) >= 0;
        }
        inline bool SetIntegerValueUnsafe(const std::string& section, const std::string& name, i64 val) {
            return m_Ini.SetLongValue(section.c_str(), name.c_str(), val) >= 0;
        }
        inline bool SetDecimalValueUnsafe(const std::string& section, const std::string& name, f64 val) {
            return m_Ini.SetDoubleValue(section.c_str(), name.c_str(), val) >= 0;
        }
        inline bool SetStringValueUnsafe(const std::string& section, const std::string& name, const char* val) {
            return m_Ini.SetValue(section.c_str(), name.c_str(), val) >= 0;
        }

        static std::unique_ptr<Config> s_Instance;

        CSimpleIniA m_Ini;
        std::string m_File;

        std::mutex m_Mutex;
    };


    template <ConfigType T>
    Expected<T> Config::Get(const std::string& section, const std::string& name) {
        std::scoped_lock lock(s_Instance->m_Mutex);

        if (!s_Instance->DoesKeyExistUnsafe(section, name))
            return Expected<T>::FromException(std::invalid_argument("Can't access a non-existing element"));

        if constexpr (std::same_as<T, bool>) {
            return s_Instance->GetBoolValueUnsafe(section, name);
        } else if constexpr (ConfigIntegerType<T>) {
            return static_cast<T>(s_Instance->GetIntegerValueUnsafe(section, name));
        } else if constexpr (ConfigDecimalType<T>) {
            return static_cast<T>(s_Instance->GetDecimalValueUnsafe(section, name));
        } else if constexpr (ConfigStringType<T>) {
            if constexpr (std::same_as<T, std::string>) {
                return std::string(s_Instance->GetStringValueUnsafe(section, name));
            }
            return s_Instance->GetStringValueUnsafe(section, name);
        }
    }

    template <ConfigType T>
    bool Config::Set(const std::string& section, const std::string& name, const T& value) {
        std::scoped_lock lock(s_Instance->m_Mutex);

        if constexpr (std::same_as<T, bool>) {
            return s_Instance->SetBoolValueUnsafe(section, name, value);
        } else if constexpr (ConfigIntegerType<T>) {
            return static_cast<T>(s_Instance->SetIntegerValueUnsafe(section, name, static_cast<i64>(value)));
        } else if constexpr (ConfigDecimalType<T>) {
            return static_cast<T>(s_Instance->SetDecimalValueUnsafe(section, name, static_cast<f64>(value)));
        } else if constexpr (ConfigStringType<T>) {
            if constexpr (std::same_as<T, std::string>) {
                return s_Instance->SetStringValueUnsafe(section, name, value.c_srt());
            }
            return s_Instance->SetStringValueUnsafe(section, name, value);
        }
    }
} // namespace Axle
