#pragma once

#include "axpch.hpp"

#include "Debug/FPS.hpp"
#include "Core/Types.hpp"

namespace Axle::Debug {
    void ShowSimpleOverlay(bool* p_open, f64 DeltaTime, FPSCounter& FPSCounter);
} // namespace Axle::Debug
