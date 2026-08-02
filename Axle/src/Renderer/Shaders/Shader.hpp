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

    /**
     * RAII wrapper of an OpenGL shader program
     *
     * All the functionality of this class is NOT THREAD SAFE and should only be accessed by the render thread.
     * */
    class Shader : public RefCounted {
    public:
        /// This constructor does nothing
        Shader() = default;

        /**
         * Creates a shader program. This constructor does not check cached shaders nor does it caches, so even though
         * the shader you are looking may have already been loaded, linked and compiled, this will do the process all
         * over again. It's recommended to instead use the static Create method to have that functionality.
         *
         * @param filename File containing all shaders needed
         * */
        Shader(const std::string& filename);

        /**
         * Creates a shader program. Unlike the base constructor this method supports caching and it's the recommended
         * way of creating a shader program.
         *
         * @param filename File containing all shaders needed
         * @param checkCached Indicates wether or not to check for cached programs with the same filename. If set to
         * false this method behaves like the constructor version.
         *
         * @returns A counted reference to the program
         * */
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
        /**
         * Deallocates all used memory
         * */
        void Reset();

        /**
         * Compiles a shader and returns its id
         *
         * @param type The type of shader to compile
         * @param source A pointer to the source of the shader
         *
         * @returns A result with the shader id (given by OpenGL) if it succeeded
         * */
        static Result<u32> CompileShader(ShaderType type, const void* source);

        u32 m_ID = 0;
        ResourceManager::ManagedFileHandle m_Handle;
    };
} // namespace Axle
