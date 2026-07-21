#include "axpch.hpp"

#include <glad/gl.h>

#include "ShaderManager.hpp"

#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "Renderer/Shaders/Shader.hpp"

#include "glm/gtc/type_ptr.hpp"
#include <ShaderSource_generated.h>
#include <flatbuffers/flatbuffers.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    std::unique_ptr<ShaderManager> ShaderManager::s_Instance = nullptr;

    void ShaderManager::Init() {
        s_Instance = std::make_unique<ShaderManager>();
        AX_CORE_INFO(LogChannel::Renderer, "Shader manager has been created");
    }

    void ShaderManager::Shutdown() {
        Clear();
        AX_CORE_INFO(LogChannel::Renderer, "ShaderManager has been deleted");
    }

    u32 ShaderManager::CreateShaderImpl(const std::string& filename, ShaderType type) {
        ZoneScopedN("Create shader from file");

        // Check if the shader has already been created
        auto find = m_ShaderToID.find(Shader{.type = type, .path = filename});
        if (find != m_ShaderToID.end())
            return find->second;

        auto exp = ResourceManager::Load(filename);

        AX_ENSURE(exp.IsOk(), LogChannel::Renderer, "Couldn't open {0} shader file", filename);
        // TODO: Put an ugly default shader if it couldn't load the file

        ResourceManager::ManagedFileHandle tmpHandle = exp.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::DataConst(tmpHandle).Unwrap();

        // Flatbuffer binary
        const ShaderCollection* collection = GetShaderCollection(readGuard.Data());

        // AX_ENSURE(ShaderCollectionBufferHasIdentifier(collection),
        //           LogChannel::Renderer,
        //           "The provided file {0} is not compatible with shaders",
        //           filename);
        // TODO: Put an ugly default shader if it couldn't load the file

        TracyGpuZone("Create shader from file");

        Shader shader = {.type = type, .path = filename};
        u32 id;
        const u8* data;
        u32 size;

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
            id = glCreateShader(GL_GEOMETRY_SHADER);
            AX_ENSURE(collection->pipeline()->geometry() != nullptr,
                      LogChannel::Renderer,
                      "There is no geometry shader in the given file");
            data = collection->pipeline()->geometry()->source()->Data();
            size = collection->pipeline()->geometry()->source()->size();
        } else if (type == ShaderType::TessControl) {
            id = glCreateShader(GL_TESS_CONTROL_SHADER);
            AX_ENSURE(collection->pipeline()->tess_control() != nullptr,
                      LogChannel::Renderer,
                      "There is no tess control shader in the given file");
            data = collection->pipeline()->tess_control()->source()->Data();
            size = collection->pipeline()->tess_control()->source()->size();
        } else if (type == ShaderType::TessEval) {
            id = glCreateShader(GL_TESS_EVALUATION_SHADER);
            AX_ENSURE(collection->pipeline()->tess_eval() != nullptr,
                      LogChannel::Renderer,
                      "There is no tess evaluation shader in the given file");
            data = collection->pipeline()->tess_eval()->source()->Data();
            size = collection->pipeline()->tess_eval()->source()->size();
        } else
            AX_PANIC(LogChannel::Renderer, "Invalid shader type");

        // Compile the shader
        const GLchar* dataCon = reinterpret_cast<const GLchar*>(data);
        GLint sizeCon = static_cast<GLint>(size);
        glShaderSource(id, 1, &dataCon, &sizeCon);
        glCompileShader(id);

        // Check compilation errors
        i32 success;
        char infoLog[1024];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);

        if (success) {
            AX_CORE_TRACE(LogChannel::Renderer, "Shader {0} compiled successfully", id);
        } else {
            glGetShaderInfoLog(id, sizeof(infoLog), nullptr, infoLog);
            AX_PANIC(
                LogChannel::Renderer, "Error compiling shader {0}, from file: {1}. Log: {2}", id, filename, infoLog);
        }

        // Store to variables
        m_IDToShader[id] = shader;
        m_ShaderToID[shader] = id;
        m_IDToFileHandle[id] = std::move(tmpHandle);

        return id;
    }

    u32 ShaderManager::CreateShaderProgramImpl(u32 shader1, u32 shader2) {
        ZoneScopedN("Create shader program");
        TracyGpuZone("Cretae shader program");

        u32 id = glCreateProgram();

        // Link shaders
        glAttachShader(id, shader1);
        glAttachShader(id, shader2);
        glLinkProgram(id);

        CheckShaderProgramLinkingErrors(id);

        m_ProgramsIDs.insert(id);

        return id;
    }
    u32 ShaderManager::CreateShaderProgramImpl(u32 shader1, u32 shader2, u32 shader3) {
        ZoneScopedN("Create shader program");
        TracyGpuZone("Cretae shader program");

        u32 id = glCreateProgram();

        // Link shaders
        glAttachShader(id, shader1);
        glAttachShader(id, shader2);
        glAttachShader(id, shader3);
        glLinkProgram(id);

        CheckShaderProgramLinkingErrors(id);

        m_ProgramsIDs.insert(id);

        return id;
    }
    u32 ShaderManager::CreateShaderProgramImpl(u32 shader1, u32 shader2, u32 shader3, u32 shader4) {
        ZoneScopedN("Create shader program");
        TracyGpuZone("Cretae shader program");

        u32 id = glCreateProgram();

        // Link shaders
        glAttachShader(id, shader1);
        glAttachShader(id, shader2);
        glAttachShader(id, shader3);
        glAttachShader(id, shader4);
        glLinkProgram(id);

        CheckShaderProgramLinkingErrors(id);

        m_ProgramsIDs.insert(id);

        return id;
    }

    void ShaderManager::UseProgramImpl(u32 id) {
        TracyGpuZone("Use program");
        glUseProgram(id);
    }
    void ShaderManager::SetBoolImpl(u32 id, const std::string& name, bool value) {
        TracyGpuZone("Set bool uniform program");
        glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<i32>(value));
    }
    void ShaderManager::SetIntImpl(u32 id, const std::string& name, i32 value) {
        TracyGpuZone("Set int uniform program");
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }
    void ShaderManager::SetFloatImpl(u32 id, const std::string& name, f32 value) {
        TracyGpuZone("Set float uniform program");
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }
    void ShaderManager::SetMat4Impl(u32 id, const std::string& name, const glm::mat4& value) {
        TracyGpuZone("Set mat4 uniform program");
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    void ShaderManager::ClearImpl() {
        ZoneScopedN("Clear all shaders and programs");
        TracyGpuZone("Clear all shaders and programs");

        for (auto it = m_IDToShader.begin(); it != m_IDToShader.end(); it++) {
            glDeleteShader(it->first);
        }

        for (auto it = m_ProgramsIDs.begin(); it != m_ProgramsIDs.end(); it++) {
            glDeleteProgram(*it);
        }

        m_IDToShader.clear();
        m_ShaderToID.clear();
        m_IDToFileHandle.clear();
        m_ProgramsIDs.clear();
    }

    void ShaderManager::CheckShaderProgramLinkingErrors(u32 id) {
        TracyGpuZone("Check program linking errors");

        i32 success;
        char infoLog[1024];

        glGetProgramiv(id, GL_LINK_STATUS, &success);

        if (success) {
            AX_CORE_TRACE(LogChannel::Renderer, "Successfully linked shader program: {0}", id);
        } else {
            glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
            AX_PANIC(LogChannel::Renderer, "Error linking program {0}. Log: {1}", id, infoLog);
        }
    }
} // namespace Axle
