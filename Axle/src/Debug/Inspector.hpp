#pragma once

#include "axpch.hpp"

namespace Axle::Debug {
    class Inspector {
    public:
        void Init();
        void Draw(const char* title, bool* open);

    private:
    };
} // namespace Axle::Debug
