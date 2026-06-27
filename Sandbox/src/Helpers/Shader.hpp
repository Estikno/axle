#pragma once

#include <Axle.hpp>
#include <glm/glm.hpp>

using namespace Axle;

class Shader {
public:
    u32 ID;

    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void Use();

    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, i32 value) const;
    void SetFloat(const std::string& name, f32 value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;
};
