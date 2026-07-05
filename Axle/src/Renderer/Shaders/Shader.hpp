#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Core/Types.hpp"

namespace Axle {
    enum class ShaderType : u8 { Unknown = 0, Vertex, TessControl, TessEval, Geometry, Fragment, MaxShaderTypes };

    /**
     * Simple RAII wrapper of an OpenGL shader
     * */
    class AXLE_API Shader {
    public:
        Shader()
            : m_ID(0),
              m_Type(ShaderType::Unknown) {}
        Shader(const std::string& filename, ShaderType type);
        ~Shader();

        // Delete copy and asignment operator
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader(Shader&& other) noexcept
            : m_ID(other.m_ID),
              m_Type(other.m_Type) {
            other.m_ID = 0;
            other.m_Type = ShaderType::Unknown;
        }

        Shader& operator=(Shader&& other) noexcept;

        inline u32 GetID() const {
            return m_ID;
        }

        inline ShaderType GetType() const {
            return m_Type;
        }

    private:
        ResourceManager::ManagedFileHandle m_FileHandle;
        ShaderType m_Type;
        u32 m_ID;
    };
} // namespace Axle
