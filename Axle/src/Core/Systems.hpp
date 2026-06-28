#pragma once

#include "axpch.hpp"

#include "Application.hpp"
#include "Events/EventHandler.hpp"
#include "Logger/Log.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include <CoroWeaver.hpp>

namespace Axle {
    /**
     * Initializes all the needed systems for the engine
     * */
    inline void InitSystems() {
        Axle::Log::Init();
        Axle::EventHandler::Init();
        Axle::InputManager::Init();
        Axle::ResourceManager::Init();
        cw::JobSystem::Init(3);
    }

    /**
     * Shutdowns all systems
     * */
    inline void ShutdownSystems() {
        Axle::ResourceManager::ShutDown();
        Axle::InputManager::ShutDown();
        Axle::EventHandler::ShutDown();
        Axle::Log::ShutDown();
    }
} // namespace Axle
