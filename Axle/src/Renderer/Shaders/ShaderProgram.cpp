#include <glad/gl.h>

#include "ShaderProgram.hpp"

#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Shader.hpp"

#include "glm/gtc/type_ptr.hpp"

namespace Axle {
    ShaderProgram::ShaderProgram(const Shader& a, const Shader& b) {
        m_ID = glCreateProgram();

        // Link shaders
        glAttachShader(m_ID, a.GetID());
        glAttachShader(m_ID, b.GetID());
        glLinkProgram(m_ID);

        CheckLinkErrors();
    }

    ShaderProgram::ShaderProgram(const Shader& a, const Shader& b, const Shader& c) {
        m_ID = glCreateProgram();

        // Link shaders
        glAttachShader(m_ID, a.GetID());
        glAttachShader(m_ID, b.GetID());
        glAttachShader(m_ID, c.GetID());
        glLinkProgram(m_ID);

        CheckLinkErrors();
    }

    ShaderProgram::~ShaderProgram() {
        glDeleteProgram(m_ID);
    }

    void ShaderProgram::CheckLinkErrors() const {
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

    void ShaderProgram::Use() const {
        glUseProgram(m_ID);
    }

    ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
        if (this != &other) {
            glDeleteProgram(m_ID);
            m_ID = other.m_ID;
            other.m_ID = 0;
        }
        return *this;
    }

    void ShaderProgram::SetMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
} // namespace Axle
