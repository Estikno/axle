#include <glad/gl.h>
#include <Axle.hpp>

#include "glm/gtc/type_ptr.hpp"

#include "ShaderHelper.hpp"

ShaderHelper::ShaderHelper(const std::string& vertexPath, const std::string& fragmentPath) {
    // Load shader files
    ResourceManager::ManagedFileHandle VertexShaderHandle = ResourceManager::GetInstance().Load(vertexPath).Unwrap();
    ResourceManager::ManagedFileHandle FragmentShaderHandle =
        ResourceManager::GetInstance().Load(fragmentPath).Unwrap();

    ResourceManager::ReadGuard rVertex =
        std::move(ResourceManager::GetInstance().DataConst(VertexShaderHandle).Unwrap());
    const char* rVertexData = rVertex.Data();
    GLint vertexSize = (GLint) rVertex.Size();
    ResourceManager::ReadGuard rFrag =
        std::move(ResourceManager::GetInstance().DataConst(FragmentShaderHandle).Unwrap());
    const char* rFragData = rFrag.Data();
    GLint fragSize = (GLint) rFrag.Size();

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &rVertexData, &vertexSize);
    glCompileShader(vertexShader);

    i32 success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        AX_PANIC(LogChannel::Client, "ERROR::SHADER::VERTEX::COMPILATION_FAILED: {0}", infoLog);
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &rFragData, &fragSize);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        AX_PANIC(LogChannel::Client, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: {0}", infoLog);
    }

    // Create the shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        AX_PANIC(LogChannel::Client, "ERROR::SHADER::PROGRAM::LINK_FAILED: {0}", infoLog);
    }

    // We no longer need the shader objects since we linked with the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void ShaderHelper::Use() {
    glUseProgram(ID);
}

void ShaderHelper::SetBool(const std::string& name, bool value) const {
    SetInt(name, static_cast<i32>(value));
}
void ShaderHelper::SetInt(const std::string& name, i32 value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void ShaderHelper::SetFloat(const std::string& name, f32 value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void ShaderHelper::SetMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderHelper::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
