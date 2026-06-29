#include "axpch.hpp"

#include "Config.hpp"

#include <SimpleIni.h>
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    std::unique_ptr<Config> Config::s_Instance;

    void Config::Init(const std::string& file) {
        if (s_Instance != nullptr) {
            AX_CORE_WARN(LogChannel::Config,
                         "Init method of the Config manager has been called a second time. IGNORING");
            return;
        }

        s_Instance = std::make_unique<Config>();
        s_Instance->m_File = file;

        AX_ENSURE(
            s_Instance->m_Ini.LoadFile(file.c_str()) >= 0, LogChannel::Config, "Error trying to load the config file");
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
        err = m_Ini.SaveFile(m_File.c_str());

        if (err < 0)
            AX_CORE_ERROR(LogChannel::Config, "Error trying to save the config file.");
    }
} // namespace Axle
