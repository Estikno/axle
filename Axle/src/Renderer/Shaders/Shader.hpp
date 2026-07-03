#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Core/Types.hpp"

namespace Axle {
    enum class ShaderType : u8 { Unknown = 0, Vertex, TessControl, TessEval, Geometry, Fragment, MaxShaderTypes };

    /**
     * Simple RAII wrapper of a OpenGL shader
     * */
    class AXLE_API Shader {
    public:
        Shader(const std::string& filename, ShaderType type);
        ~Shader();

        inline u8 GetID() const {
            return m_ID;
        }

        inline ShaderType GetType() const {
            return m_Type;
        }

    private:
        ResourceManager::ManagedFileHandle m_FileHandle;
        ShaderType m_Type;
        u8 m_ID;
    };
} // namespace Axle
