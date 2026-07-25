#include "axpch.hpp"

#include "Camera.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"

#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/glm.hpp>

namespace Axle {
    void CameraPositionerDebug::Update(f32 deltaTime) {
        // Mouse input
        glm::vec2 offsets = InputManager::GetMousePositionOffset() * p_MouseSensitivity;

        m_Yaw -= offsets.x;
        m_Pitch += offsets.y;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        // Optional: wrap yaw to avoid float growing unbounded over a long session
        if (m_Yaw > 180.0f)
            m_Yaw -= 360.0f;
        if (m_Yaw < -180.0f)
            m_Yaw += 360.0f;

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
        glm::quat qPitch = glm::angleAxis(glm::radians(m_Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat qYaw = glm::angleAxis(glm::radians(m_Yaw), m_WorldUp);

        m_Orientation = glm::normalize(qYaw * qPitch);

        m_Forward = glm::normalize(m_Orientation * glm::vec3(0.0f, 0.0f, -1.0f));
        m_Right = glm::normalize(m_Orientation * glm::vec3(1.0f, 0.0f, 0.0f));
        m_Up = glm::normalize(m_Orientation * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void CameraPositionerMoveTo::Update(f32 deltaTime) {
        // Update position
        m_PositionCurrent += p_DampingLinear * deltaTime * (m_PositionDesired - m_PositionCurrent);

        // Normalize angles
        m_AnglesCurrent = ClipAngles(m_AnglesCurrent);
        m_AnglesDesired = ClipAngles(m_AnglesDesired);

        // Update angles
        m_AnglesCurrent -= AngleDelta(m_AnglesCurrent, m_AnglesDesired) * p_DampingEulerAngles * deltaTime;
        m_AnglesCurrent = ClipAngles(m_AnglesCurrent);

        const glm::vec3 a = glm::radians(m_AnglesCurrent);
        m_CurrentTransform = glm::translate(glm::yawPitchRoll(a.y, a.x, a.z), -m_PositionCurrent);
    }

} // namespace Axle
