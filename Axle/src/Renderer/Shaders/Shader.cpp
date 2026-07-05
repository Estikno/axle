#include <glad/gl.h>
#include "Shader.hpp"

#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Resource/ResourceManager.hpp"
#include "ShaderSource_generated.h"
#include "flatbuffers/flatbuffers.h"

namespace Axle {
    Shader::Shader(const std::string& filename, ShaderType type) {
        auto exp = ResourceManager::GetInstance().Load(filename);

        AX_ENSURE(exp.IsValid(), LogChannel::Renderer, "Couldn't open {0} shader file", filename);
        // TODO: Put an ugly default shader if it couldn't load the file

        m_FileHandle = exp.Unwrap();
        ResourceManager::ReadGuard readGuard = ResourceManager::GetInstance().DataConst(m_FileHandle).Unwrap();

        // Flatbuffer binary
        const ShaderCollection* collection = GetShaderCollection(readGuard.Data());

        // AX_ENSURE(ShaderCollectionBufferHasIdentifier(collection),
        //           LogChannel::Renderer,
        //           "The provided file {0} is not compatible with shaders",
        //           filename);
        // TODO: Put an ugly default shader if it couldn't load the file

        m_Type = type;
        const u8* data;
        u32 size;

        // Create the shader
        if (type == ShaderType::Vertex) {
            m_ID = glCreateShader(GL_VERTEX_SHADER);
            data = collection->pipeline()->vertex()->source()->Data();
            size = collection->pipeline()->vertex()->source()->size();
        } else if (type == ShaderType::Fragment) {
            m_ID = glCreateShader(GL_FRAGMENT_SHADER);
            data = collection->pipeline()->fragment()->source()->Data();
            size = collection->pipeline()->fragment()->source()->size();
        } else if (type == ShaderType::Geometry) {
            m_ID = glCreateShader(GL_GEOMETRY_SHADER);
            AX_ENSURE(collection->pipeline()->geometry() != nullptr,
                      LogChannel::Renderer,
                      "There is no geometry shader in the given file");
            data = collection->pipeline()->geometry()->source()->Data();
            size = collection->pipeline()->geometry()->source()->size();
        } else if (type == ShaderType::TessControl) {
            m_ID = glCreateShader(GL_TESS_CONTROL_SHADER);
            AX_ENSURE(collection->pipeline()->tess_control() != nullptr,
                      LogChannel::Renderer,
                      "There is no tess control shader in the given file");
            data = collection->pipeline()->tess_control()->source()->Data();
            size = collection->pipeline()->tess_control()->source()->size();
        } else if (type == ShaderType::TessEval) {
            m_ID = glCreateShader(GL_TESS_EVALUATION_SHADER);
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
        glShaderSource(m_ID, 1, &dataCon, &sizeCon);
        glCompileShader(m_ID);

        // Check compilation errors
        i32 success;
        char infoLog[1024];
        glGetShaderiv(m_ID, GL_COMPILE_STATUS, &success);

        if (success) {
            AX_CORE_TRACE(LogChannel::Renderer, "Shader {0} compiled successfully", m_ID);
        } else {
            glGetShaderInfoLog(m_ID, sizeof(infoLog), nullptr, infoLog);
            AX_PANIC(LogChannel::Renderer, "Error compiling shader {0}. Log: {1}", m_ID, infoLog);
        }
    }

    Shader::~Shader() {
        glDeleteShader(m_ID);
    }

    Shader& Shader::operator=(Shader&& other) noexcept {
        if (this != &other) {
            glDeleteShader(m_ID);
            m_ID = other.m_ID;
            m_Type = other.m_Type;

            other.m_ID = 0;
            other.m_Type = ShaderType::Unknown;
        }
        return *this;
    }
} // namespace Axle
