#include "axpch.hpp"

#include "Config.hpp"

#include <SimpleIni.h>
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    struct Config::Impl {
        Impl()
            : ini(false, false, false) {}

        CSimpleIniA ini;
    };

    std::unique_ptr<Config> Config::s_Instance = nullptr;

    Config::Config()
        : m_Impl(std::make_unique<Impl>()) {}
    Config::~Config() = default;

    void Config::Init(const std::string& file) {
        if (s_Instance != nullptr) {
            AX_CORE_WARN(LogChannel::Config,
                         "Init method of the Config manager has been called a second time. IGNORING");
            return;
        }

        s_Instance = std::make_unique<Config>();
        s_Instance->m_File = file;

        if (s_Instance->m_Impl->ini.LoadFile(file.c_str()) < 0)
            AX_CORE_WARN(LogChannel::Config, "Error trying to load the config file");

        AX_CORE_INFO(LogChannel::Config, "Config manager initialized...");
    }

    void Config::ShutDown() {
        Save();

        s_Instance.reset();
        AX_CORE_INFO(LogChannel::Config, "Config manager deleted...");
    }

    void Config::SaveImpl() {
        std::scoped_lock lock(m_Mutex);

        SI_Error err;
        err = m_Impl->ini.SaveFile(m_File.c_str());

        if (err < 0)
            AX_CORE_ERROR(LogChannel::Config, "Error trying to save the config file.");
    }


    bool Config::DeleteImpl(const char* section, const char* name) {
        std::scoped_lock lock(s_Instance->m_Mutex);
        return m_Impl->ini.Delete(section, name);
    }

    // Unsafe functions
    bool Config::DoesKeyExistUnsafe(const std::string& section, const std::string& name) {
        return m_Impl->ini.GetValue(section.c_str(), name.c_str()) != nullptr;
    }

    // Specialized getters and setters
    bool Config::GetBoolValueUnsafe(const std::string& section, const std::string& name) {
        return m_Impl->ini.GetBoolValue(section.c_str(), name.c_str());
    }
    i64 Config::GetIntegerValueUnsafe(const std::string& section, const std::string& name) {
        return static_cast<i64>(m_Impl->ini.GetLongValue(section.c_str(), name.c_str()));
    }
    f64 Config::GetDecimalValueUnsafe(const std::string& section, const std::string& name) {
        return m_Impl->ini.GetDoubleValue(section.c_str(), name.c_str());
    }
    const char* Config::GetStringValueUnsafe(const std::string& section, const std::string& name) {
        return m_Impl->ini.GetValue(section.c_str(), name.c_str());
    }

    bool Config::SetBoolValueUnsafe(const std::string& section, const std::string& name, bool val) {
        return m_Impl->ini.SetBoolValue(section.c_str(), name.c_str(), val) >= 0;
    }
    bool Config::SetIntegerValueUnsafe(const std::string& section, const std::string& name, i64 val) {
        return m_Impl->ini.SetLongValue(section.c_str(), name.c_str(), val) >= 0;
    }
    bool Config::SetDecimalValueUnsafe(const std::string& section, const std::string& name, f64 val) {
        return m_Impl->ini.SetDoubleValue(section.c_str(), name.c_str(), val) >= 0;
    }
    bool Config::SetStringValueUnsafe(const std::string& section, const std::string& name, const char* val) {
        return m_Impl->ini.SetValue(section.c_str(), name.c_str(), val) >= 0;
    }
} // namespace Axle
