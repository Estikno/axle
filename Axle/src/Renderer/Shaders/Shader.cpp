#include "axpch.hpp"

#include <glad/gl.h>

#include "Shader.hpp"
#include "ShaderManager.hpp"
#include "Renderer/GLDebug.hpp"
#include "Core/Error/Error.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Error/Result.hpp"
#include "Core/Logger/Log.hpp"

#include <ShaderSource_generated.h>
#include <flatbuffers/flatbuffers.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "glm/gtc/type_ptr.hpp"

namespace Axle {
    u32 ShaderDataTypeSize(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:
                return 4;
            case ShaderDataType::Vec2:
                return 4 * 2;
            case ShaderDataType::Vec3:
                return 4 * 3;
            case ShaderDataType::Vec4:
                return 4 * 4;
            case ShaderDataType::Mat3:
                return 4 * 3 * 3;
            case ShaderDataType::Mat4:
                return 4 * 4 * 4;
            case ShaderDataType::Int:
                return 4;
            case ShaderDataType::Int2:
                return 4 * 2;
            case ShaderDataType::Int3:
                return 4 * 3;
            case ShaderDataType::Int4:
                return 4 * 4;
            case ShaderDataType::Bool:
                return 4;
        }

        AX_PANIC(LogChannel::Renderer, "Unknown ShaderDataType!");
    }

    u32 ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:
                return GL_FLOAT;
            case ShaderDataType::Vec2:
                return GL_FLOAT;
            case ShaderDataType::Vec3:
                return GL_FLOAT;
            case ShaderDataType::Vec4:
                return GL_FLOAT;
            case ShaderDataType::Mat3:
                return GL_FLOAT;
            case ShaderDataType::Mat4:
                return GL_FLOAT;
            case ShaderDataType::Int:
                return GL_INT;
            case ShaderDataType::Int2:
                return GL_INT;
            case ShaderDataType::Int3:
                return GL_INT;
            case ShaderDataType::Int4:
                return GL_INT;
            case ShaderDataType::Bool:
                return GL_BOOL;
        }

        AX_PANIC(LogChannel::Renderer, "Unknown ShaderDataType!");
    }

    Shader::Shader(const std::string& filename, bool checkCached) {
        ZoneScopedN("Create shader from file");

        // TODO: Check if the shader has already been created

        auto exp = ResourceManager::Load(filename);
        AX_ENSURE(exp.IsOk(), LogChannel::Renderer, "Couldn't open {0} shader file", filename);
        // TODO: Put an ugly default shader if it couldn't load the file

        m_Handle = exp.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::DataConst(m_Handle).Unwrap();

        // Flatbuffer binary
        const ShaderCollection* collection = GetShaderCollection(readGuard.Data());

        // AX_ENSURE(ShaderCollectionBufferHasIdentifier(collection),
        //           LogChannel::Renderer,
        //           "The provided file {0} is not compatible with shaders",
        //           filename);
        // TODO: Put an ugly default shader if it couldn't load the file

        TracyGpuZone("Create shader from file");

        m_ID = glCreateProgram();
        std::vector<u32> shaderIDs;

        for (u8 i = 1; i < static_cast<u8>(ShaderType::MaxShaderTypes); ++i) {
            Result<u32> res = CompileShader(static_cast<ShaderType>(i), collection);

            if (res.IsOk()) {
                AX_GL_CALL(glAttachShader(m_ID, res.Unwrap()));
                shaderIDs.push_back(res.Unwrap());
            } else if (static_cast<ShaderType>(i) == ShaderType::Vertex ||
                       static_cast<ShaderType>(i) == ShaderType::Fragment) {
                AX_PANIC(LogChannel::Renderer, res.UnwrapErr().message);
            } else {
                AX_CORE_WARN(LogChannel::Renderer, res.UnwrapErr().message);
            }
        }

        AX_GL_CALL(glLinkProgram(m_ID));
        for (u32& id : shaderIDs) {
            AX_GL_CALL(glDeleteShader(id));
        }

#ifdef AX_DEBUG
        // Check linking errors
        i32 success;
        char infoLog[1024];

        AX_GL_CALL(glGetProgramiv(m_ID, GL_LINK_STATUS, &success));

        if (success) {
            AX_CORE_TRACE(LogChannel::Renderer, "Successfully linked shader program: {0}", m_ID);
        } else {
            AX_GL_CALL(glGetProgramInfoLog(m_ID, sizeof(infoLog), nullptr, infoLog));
            AX_PANIC(LogChannel::Renderer, "Error linking program {0}. Log: {1}", m_ID, infoLog);
        }
#endif // AX_DEBUG

        // TODO: Add shader entry to shader manager for cache
    }

    Result<u32> Shader::CompileShader(ShaderType type, const void* source) {
        // Flatbuffer binary
        const ShaderCollection* collection = (ShaderCollection*) source;
        const u8* data;
        u32 size;
        u32 id;

        // Create the shader
        if (type == ShaderType::Vertex) {
            id = glCreateShader(GL_VERTEX_SHADER);
            data = collection->pipeline()->vertex()->source()->Data();
            size = collection->pipeline()->vertex()->source()->size();
        } else if (type == ShaderType::Fragment) {
            id = glCreateShader(GL_FRAGMENT_SHADER);
            data = collection->pipeline()->fragment()->source()->Data();
            size = collection->pipeline()->fragment()->source()->size();
        } else if (type == ShaderType::Geometry) {
            if (collection->pipeline()->geometry() == nullptr) {
                return Result<u32>::Err(Error(ErrorCode::NotFound, "There is no geometry shader in the given file"));
            }
            data = collection->pipeline()->geometry()->source()->Data();
            size = collection->pipeline()->geometry()->source()->size();
        } else if (type == ShaderType::TessControl) {
            if (collection->pipeline()->tess_control() == nullptr) {
                return Result<u32>::Err(
                    Error(ErrorCode::NotFound, "There is no tess control shader in the given file"));
            }
            id = glCreateShader(GL_TESS_CONTROL_SHADER);
            data = collection->pipeline()->tess_control()->source()->Data();
            size = collection->pipeline()->tess_control()->source()->size();
        } else if (type == ShaderType::TessEval) {
            if (collection->pipeline()->tess_eval() == nullptr) {
                return Result<u32>::Err(Error(ErrorCode::NotFound, "There is no tess eval shader in the given file"));
            }
            id = glCreateShader(GL_TESS_EVALUATION_SHADER);
            data = collection->pipeline()->tess_eval()->source()->Data();
            size = collection->pipeline()->tess_eval()->source()->size();
        } else {
            AX_PANIC(LogChannel::Renderer, "Invalid shader type");
        }

        // Compile the shader
        const GLchar* dataCon = reinterpret_cast<const GLchar*>(data);
        GLint sizeCon = static_cast<GLint>(size);
        AX_GL_CALL(glShaderSource(id, 1, &dataCon, &sizeCon));
        AX_GL_CALL(glCompileShader(id));

        // Check compilation errors
#ifdef AX_DEBUG
        i32 success;
        char infoLog[1024];
        AX_GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &success));

        if (success) {
            AX_CORE_TRACE(LogChannel::Renderer, "Shader {0} compiled successfully", id);
        } else {
            AX_GL_CALL(glGetShaderInfoLog(id, sizeof(infoLog), nullptr, infoLog));
            return Result<u32>::Err(
                Error(ErrorCode::ShaderCompileFailed,
                      "Error compiling shader " + std::to_string(id) + ". Log: " + std::string(infoLog)));
        }
#endif // AX_DEBUG

        return id;
    }

    Shader::~Shader() {
        Reset();
    }

    Shader::Shader(Shader&& other) noexcept
        : m_ID(other.m_ID),
          m_Handle(std::move(other.m_Handle)) {
        other.m_ID = 0;
    }

    Shader& Shader::operator=(Shader&& other) noexcept {
        if (this != &other) {
            Reset();

            m_ID = other.m_ID;
            m_Handle = std::move(other.m_Handle);

            other.m_ID = 0;
        }
        return *this;
    }

    void Shader::Reset() {
        if (m_ID != 0)
            AX_GL_CALL(glDeleteProgram(m_ID));
    }

    void Shader::Use() {
        TracyGpuZone("Use program");
        AX_GL_CALL(glUseProgram(m_ID));
    }

    void Shader::SetBoolUniform(const std::string& name, bool value) {
        TracyGpuZone("Set bool uniform program");
        AX_GL_CALL(glUniform1i(glGetUniformLocation(m_ID, name.c_str()), static_cast<i32>(value)));
    }

    void Shader::SetIntUniform(const std::string& name, i32 value) {
        TracyGpuZone("Set int uniform program");
        AX_GL_CALL(glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value));
    }

    void Shader::SetFloatUniform(const std::string& name, f32 value) {
        TracyGpuZone("Set float uniform program");
        AX_GL_CALL(glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value));
    }

    void Shader::SetMat4Uniform(const std::string& name, const glm::mat4& value) {
        TracyGpuZone("Set mat4 uniform program");
        AX_GL_CALL(glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)));
    }
} // namespace Axle
