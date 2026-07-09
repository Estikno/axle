#include "glm/trigonometric.hpp"
#include <glad/gl.h>
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"

#include "Camera.hpp"

void Camera::ProcessKeyboard(f32 deltaTime) {
    f32 velocity = m_MoveSpeed * deltaTime;

    if (InputManager::GetKey(Keys::W))
        m_Position += m_Front * velocity;
    if (InputManager::GetKey(Keys::S))
        m_Position -= m_Front * velocity;
    if (InputManager::GetKey(Keys::A))
        m_Position -= m_Right * velocity;
    if (InputManager::GetKey(Keys::D))
        m_Position += m_Right * velocity;
    if (InputManager::GetKey(Keys::Space))
        m_Position += m_WorldUp * velocity;
    if (InputManager::GetKey(Keys::C))
        m_Position -= m_WorldUp * velocity;
}

void Camera::ProcessMouseMovement(f32 deltaTime, bool constrainPitch) {
    glm::vec2 offsets = InputManager::GetMousePositionOffset();

    offsets.x *= m_MouseSensitivity * deltaTime;
    offsets.y *= m_MouseSensitivity * deltaTime;

    m_Yaw += offsets.x;
    m_Pitch += offsets.y;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;
    }

    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(f32 yOffset) {
    m_Zoom -= yOffset;

    if (m_Zoom < 1.0f)
        m_Zoom = 1.0f;
    if (m_Zoom > 45.0f)
        m_Zoom = 45.0f;
}

void Camera::UpdateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
