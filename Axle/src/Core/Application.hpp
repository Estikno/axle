#pragma once

#include "Core.hpp"

namespace Axle {
    class AXLE_API Application {
      public:
        Application();
        virtual ~Application();

        void Run();
    };

    // To be defined in client
    Application *CreateApplication();
} 
