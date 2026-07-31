#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Core/Error/Result.hpp"

#include <glm/glm.hpp>

namespace Axle {
    enum class ShaderType : u8 { Unknown = 0, Vertex, TessControl, TessEval, Geometry, Fragment, MaxShaderTypes };
    enum class ShaderDataType : u8 { None = 0, Float, Vec2, Vec3, Vec4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool };

    u32 ShaderDataTypeSize(ShaderDataType type);
    u32 ShaderDataTypeToOpenGLBaseType(ShaderDataType type);

    class Shader : public RefCounted {
    public:
        Shader() = default;
        Shader(const std::string& filename);

        static Ref<Shader> Create(const std::string& filename, bool checkCached = true);

        ~Shader();

        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        void Use() const;

        inline u32 GetID() const {
            return m_ID;
        }

        void SetBoolUniform(const std::string& name, bool value) const;
        void SetIntUniform(const std::string& name, i32 value) const;
        void SetFloatUniform(const std::string& name, f32 value) const;
        void SetMat4Uniform(const std::string& name, const glm::mat4& value) const;

    private:
        void Reset();

        static Result<u32> CompileShader(ShaderType type, const void* source);

        u32 m_ID = 0;
        ResourceManager::ManagedFileHandle m_Handle;
    };
} // namespace Axle
