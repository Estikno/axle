#pragma once

#include "axpch.hpp"

#include "Other/CustomTypes/Expected.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Shader.hpp"

#include "glm/ext/matrix_float4x4.hpp"

namespace Axle {
    class AXLE_API ShaderProgram {
    public:
        ShaderProgram()
            : m_ID(0) {}
        ShaderProgram(const Shader& a, const Shader& b);
        ShaderProgram(const Shader& a, const Shader& b, const Shader& c);
        // TODO: Add more constructors that take more shaders
        ~ShaderProgram();

        // Delete copy and asignment operator
        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        ShaderProgram(ShaderProgram&& other) noexcept
            : m_ID(other.m_ID) {
            other.m_ID = 0;
        }
        ShaderProgram& operator=(ShaderProgram&& other) noexcept;

        void SetMat4(const std::string& name, const glm::mat4& mat) const;

        /**
         * Tells OpenGL to use the program.
         * */
        void Use() const;

        /**
         * Gets the underlying OpenGL identifier of the program.
         * */
        inline u32 GetID() const {
            return m_ID;
        }

    private:
        /**
         * Checks for any linking errors.
         * */
        void CheckLinkErrors() const;

        u32 m_ID;
    };
} // namespace Axle
