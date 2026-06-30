#pragma once

#include <Axle.hpp>

#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>

using namespace Axle;

constexpr f32 YAW = -90.0f;
constexpr f32 PITCH = 0.0f;
constexpr f32 SPEED = 2.5f;
constexpr f32 SENSITIVITY = 15.0f;
constexpr f32 ZOOM = 45.0f;

class Camera {
public:
    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW,
           float pitch = PITCH)
        : m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          m_MoveSpeed(SPEED),
          m_MouseSensitivity(SENSITIVITY),
          m_Zoom(ZOOM) {
        m_Position = position;
        m_WorldUp = up;
        m_Yaw = yaw;
        m_Pitch = pitch;

        UpdateCameraVectors();
    }

    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          m_MoveSpeed(SPEED),
          m_MouseSensitivity(SENSITIVITY),
          m_Zoom(ZOOM) {
        m_Position = glm::vec3(posX, posY, posZ);
        m_WorldUp = glm::vec3(upX, upY, upZ);
        m_Yaw = yaw;
        m_Pitch = pitch;

        UpdateCameraVectors();
    }

    inline glm::mat4 GetViewMatrix() const {
        return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }

    void ProcessKeyboard(f32 deltaTime);
    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(f32 deltaTime, bool constrainPitch = true);
    void ProcessMouseScroll(f32 yOffset);

    inline f32 GetZoom() const {
        return m_Zoom;
    }

    inline glm::vec3 GetPosition() const {
        return m_Position;
    }

    inline glm::vec3 GetFront() const {
        return m_Front;
    }

private:
    void UpdateCameraVectors();

    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    f32 m_Yaw, m_Pitch;

    f32 m_MoveSpeed, m_MouseSensitivity, m_Zoom;
};
