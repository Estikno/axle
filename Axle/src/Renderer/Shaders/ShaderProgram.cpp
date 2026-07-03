#include <glad/gl.h>

#include "ShaderProgram.hpp"

#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Shader.hpp"

namespace Axle {
    ShaderProgram::ShaderProgram(const Shader& a, const Shader& b) {
        m_ID = glCreateProgram();

        // Link shaders
        glAttachShader(m_ID, a.GetID());
        glAttachShader(m_ID, b.GetID());
        glLinkProgram(m_ID);

        // Check linking errors
        i32 success;
        char infoLog[1024];

        glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

        if (success) {
            AX_CORE_TRACE(LogChannel::Renderer, "Successfully linked shader program: {0}", m_ID);
        } else {
            glGetProgramInfoLog(m_ID, sizeof(infoLog), nullptr, infoLog);
            AX_PANIC(LogChannel::Renderer, "Error linking program {0}. Log: {1}", m_ID, infoLog);
        }
    }

    ShaderProgram::~ShaderProgram() {
        glDeleteProgram(m_ID);
    }

    void ShaderProgram::Use() const {
        glUseProgram(m_ID);
    }
} // namespace Axle
