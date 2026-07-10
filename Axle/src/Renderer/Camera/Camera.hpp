#pragma once

#include "axpch.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Config/Config.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>

namespace Axle {
    class ICameraPositioner {
    public:
        virtual ~ICameraPositioner() = default;
        virtual glm::mat4 GetViewMatrix() const = 0;
        virtual glm::vec3 GetPosition() const = 0;
    };

    class AXLE_API Camera final {
    public:
        explicit Camera()
            : m_Positioner(nullptr) {}

        explicit Camera(ICameraPositioner& positioner)
            : m_Positioner(&positioner) {}

        Camera(const Camera&) = default;
        Camera& operator=(const Camera&) = default;

        inline glm::mat4 GetViewMatrix() const {
            return m_Positioner->GetViewMatrix();
        }

        inline glm::vec3 GetPosition() const {
            return m_Positioner->GetPosition();
        }

    private:
        const ICameraPositioner* m_Positioner;
    };


    class AXLE_API CameraPositionerDebug final : public ICameraPositioner {
    public:
        CameraPositionerDebug()
            : CameraPositionerDebug(glm::vec3(0.0f), -90.0f, 0.0f) {
            InitPublicVariables();
            UpdateCameraVectors();
        }

        CameraPositionerDebug(const glm::vec3& pos, f32 yaw, f32 pitch)
            : m_Position(pos),
              m_Up(glm::vec3(0.0f, 1.0f, 0.0f)),
              m_Forward(glm::vec3(0.0f, 0.0f, -1.0f)),
              m_Right(glm::vec3(1.0f, 0.0f, 0.0f)),
              m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
              m_Yaw(yaw),
              m_Pitch(pitch) {
            InitPublicVariables();
            UpdateCameraVectors();
        }

        void Update(f32 deltaTime);
        void ProcessMouseScroll(f32 yOffset);

        inline virtual glm::vec3 GetPosition() const override {
            return m_Position;
        }

        inline virtual glm::mat4 GetViewMatrix() const override {
            return glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
        }

        inline f32 GetFOV() const {
            return m_FOV;
        }

        f32 p_MouseSensitivity;
        f32 p_MoveSpeed;
        f32 p_MinFOV;
        f32 p_MaxFOV;

    private:
        inline static constexpr const char* ConfigSection = "DebugCamera";

        inline void InitPublicVariables() {
            p_MouseSensitivity = Config::GetOrSet(std::string(ConfigSection), "MouseSensitivity", 4.0f);
            p_MoveSpeed = Config::GetOrSet(std::string(ConfigSection), "MoveSpeed", 10.0f);
            p_MinFOV = Config::GetOrSet(std::string(ConfigSection), "MinFOV", 1.0f);
            p_MaxFOV = Config::GetOrSet(std::string(ConfigSection), "MaxFOV", 45.0f);
            m_FOV = p_MaxFOV;
        }

        void UpdateCameraVectors();

        glm::vec3 m_Position;
        glm::vec3 m_Forward;
        glm::vec3 m_Right;
        glm::vec3 m_Up;
        glm::vec3 m_WorldUp;

        f32 m_Yaw, m_Pitch, m_FOV;
    };
} // namespace Axle
