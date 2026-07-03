#pragma once

#include <Axle.hpp>
#include <glm/glm.hpp>

using namespace Axle;

class ShaderHelper {
public:
    u32 ID;

    ShaderHelper() = default;
    ShaderHelper(const std::string& vertexPath, const std::string& fragmentPath);
    void Use();

    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, i32 value) const;
    void SetFloat(const std::string& name, f32 value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    inline void SetVec3(const std::string& name, f32 x, f32 y, f32 z) const {
        SetVec3(name, glm::vec3(x, y, x));
    }
};
