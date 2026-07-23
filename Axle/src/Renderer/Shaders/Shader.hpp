#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Core/Types.hpp"

namespace Axle {
    enum class ShaderType : u8 { Unknown = 0, Vertex, TessControl, TessEval, Geometry, Fragment, MaxShaderTypes };
    enum class ShaderDataType : u8 { None = 0, Float, Vec2, Vec3, Vec4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool };

    struct Shader {
        ShaderType type;
        std::string path;

        bool operator==(const Shader& other) const {
            return type == other.type && path == other.path;
        }
    };

    u32 ShaderDataTypeSize(ShaderDataType type);
    u32 ShaderDataTypeToOpenGLBaseType(ShaderDataType type);

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
