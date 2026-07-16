#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Core/Types.hpp"

namespace Axle {
    enum class ShaderType : u8 { Unknown = 0, Vertex, TessControl, TessEval, Geometry, Fragment, MaxShaderTypes };

    struct Shader {
        ShaderType type;
        std::string path;

        bool operator==(const Shader& other) const {
            return type == other.type && path == other.path;
        }
    };

    /**
     * Simple RAII wrapper of an OpenGL shader
     * */
    // class AXLE_API Shader {
    // public:
    //     Shader()
    //         : m_ID(0),
    //           m_Type(ShaderType::Unknown) {}
    //     Shader(const std::string& filename, ShaderType type);
    //     ~Shader();
    //
    //     // Delete copy and asignment operator
    //     Shader(const Shader&) = delete;
    //     Shader& operator=(const Shader&) = delete;
    //
    //     Shader(Shader&& other) noexcept
    //         : m_ID(other.m_ID),
    //           m_Type(other.m_Type) {
    //         other.m_ID = 0;
    //         other.m_Type = ShaderType::Unknown;
    //     }
    //
    //     Shader& operator=(Shader&& other) noexcept;
    //
    //     /**
    //      * Gets the underlying OpenGL identifier.
    //      * */
    //     inline u32 GetID() const {
    //         return m_ID;
    //     }
    //
    //     /**
    //      * Whats the shader's type?
    //      * */
    //     inline ShaderType GetType() const {
    //         return m_Type;
    //     }
    //
    // private:
    //     ResourceManager::ManagedFileHandle m_FileHandle;
    //     ShaderType m_Type;
    //     u32 m_ID;
    // };
} // namespace Axle

namespace std {
    template <>
    struct hash<Axle::Shader> {
        size_t operator()(const Axle::Shader& s) const noexcept {
            size_t h1 = std::hash<int>{}(static_cast<int>(s.type));
            size_t h2 = std::hash<std::string>{}(s.path);
            // combine the two hashes
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
} // namespace std
