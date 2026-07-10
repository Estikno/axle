#include "axpch.hpp"

#include "Camera.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"

#include <glm/glm.hpp>

namespace Axle {
    void CameraPositionerDebug::Update(f32 deltaTime) {
        // Mouse input
        glm::vec2 offsets = InputManager::GetMousePositionOffset() * p_MouseSensitivity;

        m_Yaw += offsets.x;
        m_Pitch += offsets.y;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        UpdateCameraVectors();

        // Key input
        f32 velocity = p_MoveSpeed * deltaTime;

        if (InputManager::GetKey(Keys::W))
            m_Position += m_Forward * velocity;
        if (InputManager::GetKey(Keys::S))
            m_Position -= m_Forward * velocity;

        if (InputManager::GetKey(Keys::A))
            m_Position -= m_Right * velocity;
        if (InputManager::GetKey(Keys::D))
            m_Position += m_Right * velocity;

        if (InputManager::GetKey(Keys::Space))
            m_Position += m_WorldUp * velocity;
        if (InputManager::GetKey(Keys::C))
            m_Position -= m_WorldUp * velocity;
    }

    void CameraPositionerDebug::ProcessMouseScroll(f32 yOffset) {
        m_FOV -= yOffset;

        if (m_FOV < p_MinFOV)
            m_FOV = p_MinFOV;
        if (m_FOV > p_MaxFOV)
            m_FOV = p_MaxFOV;
    }

    void CameraPositionerDebug::UpdateCameraVectors() {
        glm::vec3 forward;
        forward.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        forward.y = sin(glm::radians(m_Pitch));
        forward.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Forward = glm::normalize(forward);

        m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
        m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
    }

} // namespace Axle
