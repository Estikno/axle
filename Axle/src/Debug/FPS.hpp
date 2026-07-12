#pragma once

#include "axpch.hpp"

#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Types.hpp"

namespace Axle::Debug {
    class FPSCounter {
    public:
        FPSCounter(f32 avgInterval)
            : m_AvgInterval(avgInterval) {
            AX_ASSERT(avgInterval > 0.0f, LogChannel::Debug, "The fps average interval must be greater than 0");
        }

        bool Tick(f32 deltaTime, bool frameRendered = true) {
            if (frameRendered)
                m_NumFrames++;
            m_AccumulatedTime += deltaTime;

            if (m_AccumulatedTime > m_AvgInterval) {
                m_CurrentFPS = static_cast<f32>(m_NumFrames / m_AccumulatedTime);
                m_NumFrames = 0;
                m_AccumulatedTime = 0.0;
                return true;
            }

            return false;
        }

        inline f32 GetFPS() const {
            return m_CurrentFPS;
        }

    private:
        const f32 m_AvgInterval = 0.5f;
        u32 m_NumFrames = 0;
        f64 m_AccumulatedTime = 0.0;
        f32 m_CurrentFPS = 0.0f;
    };
} // namespace Axle::Debug
