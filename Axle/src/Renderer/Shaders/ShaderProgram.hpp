#pragma once

#include "axpch.hpp"

#include "Other/CustomTypes/Expected.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Shader.hpp"

namespace Axle {
    class AXLE_API ShaderProgram {
    public:
        ShaderProgram(const Shader& a, const Shader& b);
        // TODO: Add more constructors that take more shaders
        ~ShaderProgram();

        void Use() const;
        inline u8 GetID() const {
            return m_ID;
        }

    private:
        u8 m_ID;
    };
} // namespace Axle
