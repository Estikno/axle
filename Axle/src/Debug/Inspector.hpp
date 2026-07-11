#pragma once

#include "axpch.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle::Debug {
    class Inspector {
    public:
        static void Init();
        static void Shutdown();

        Inspector() = default;
        ~Inspector() = default;

        static void Draw(const char* title, bool* open);

        inline static void AddHeader(std::function<void()> f) {
            AX_ASSERT(
                s_Instance != nullptr, LogChannel::Debug, "The init method of the Inspector must have been called");
            GetInstance().m_Headers.push_back(f);
        }

        inline static Inspector& GetInstance() {
            return *s_Instance;
        }

    private:
        static std::unique_ptr<Inspector> s_Instance;

        std::vector<std::function<void()>> m_Headers;
    };
} // namespace Axle::Debug
