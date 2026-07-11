#pragma once

#include "axpch.hpp"
#include "Core/Core.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Types.hpp"
#include "Core/Config/Config.hpp"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>

namespace Axle {
    class ICameraPositioner {
    public:
        virtual ~ICameraPositioner() = default;
        virtual glm::mat4 GetViewMatrix() const = 0;
        virtual glm::vec3 GetPosition() const = 0;
        virtual void Update(f32 deltaTime) = 0;
    };

    class AXLE_API Camera final {
    public:
        explicit Camera()
            : m_Positioner(nullptr),
              m_DeletePositioner(true) {}

        explicit Camera(ICameraPositioner& positioner, bool deletePositioner)
            : m_Positioner(&positioner),
              m_DeletePositioner(deletePositioner) {}

        explicit Camera(ICameraPositioner* positioner, bool deletePositioner)
            : m_Positioner(positioner),
              m_DeletePositioner(deletePositioner) {}

        ~Camera() {
            if (!m_DeletePositioner)
                return;

            ICameraPositioner* old = ExchangePositioner(nullptr);
            if (old != nullptr)
                delete old;
        }

        // Camera(const Camera&) = default;
        // Camera& operator=(const Camera&) = default;

        inline glm::mat4 GetViewMatrix() const {
            AX_ASSERT(m_Positioner.load(std::memory_order_acquire) != nullptr,
                      LogChannel::Renderer,
                      "Must add a positioner before calling any method.");
            return m_Positioner.load(std::memory_order_acquire)->GetViewMatrix();
        }

        inline glm::vec3 GetPosition() const {
            AX_ASSERT(m_Positioner.load(std::memory_order_acquire) != nullptr,
                      LogChannel::Renderer,
                      "Must add a positioner before calling any method.");
            return m_Positioner.load(std::memory_order_acquire)->GetPosition();
        }

        inline void ChangePositioner(ICameraPositioner& newPositioner) {
            m_Positioner.store(&newPositioner, std::memory_order_release);
        }

        inline void ChangePositioner(ICameraPositioner* newPositioner) {
            m_Positioner.store(newPositioner, std::memory_order_release);
        }

        inline ICameraPositioner* ExchangePositioner(ICameraPositioner& newPositioner) {
            return m_Positioner.exchange(&newPositioner, std::memory_order_acq_rel);
        }

        inline ICameraPositioner* ExchangePositioner(ICameraPositioner* newPositioner) {
            return m_Positioner.exchange(newPositioner, std::memory_order_acq_rel);
        }

        inline ICameraPositioner* GetPositioner() {
            return m_Positioner.load(std::memory_order_acquire);
        }

        inline void SetDeletePolicy(bool policy) {
            m_DeletePositioner.store(policy, std::memory_order_release);
        }

    private:
        static_assert(std::atomic<ICameraPositioner*>::is_always_lock_free,
                      "Positioner pointer is not always lock free");
        std::atomic<ICameraPositioner*> m_Positioner = nullptr;
        std::atomic_bool m_DeletePositioner = true;
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

        virtual void Update(f32 deltaTime) override;
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

    class AXLE_API CameraPositionerMoveTo final : public ICameraPositioner {
    public:
        CameraPositionerMoveTo()
            : CameraPositionerMoveTo(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)) {}

        CameraPositionerMoveTo(const glm::vec3& pos, const glm::vec3& angles)
            : m_PositionCurrent(pos),
              m_PositionDesired(pos),
              m_AnglesCurrent(angles),
              m_AnglesDesired(angles) {}

        virtual void Update(f32 deltaTime) override;

        inline void SetPosition(const glm::vec3& pos) {
            m_PositionCurrent = pos;
        }
        inline void SetAngles(f32 pitch, f32 yaw, f32 roll) {
            m_AnglesCurrent = glm::vec3(pitch, yaw, roll);
        }
        inline void SetAngles(const glm::vec3& angles) {
            m_AnglesCurrent = angles;
        }
        inline void SetDesiredPosition(const glm::vec3& pos) {
            m_PositionDesired = pos;
        }
        inline void SetDesiredAngles(f32 pitch, f32 yaw, f32 roll) {
            m_AnglesDesired = glm::vec3(pitch, yaw, roll);
        }
        inline void SetDesiredAngles(const glm::vec3& angles) {
            m_AnglesDesired = angles;
        }

        inline virtual glm::vec3 GetPosition() const override {
            return m_PositionCurrent;
        }
        inline virtual glm::mat4 GetViewMatrix() const override {
            return m_CurrentTransform;
        }

        f32 p_DampingLinear;
        glm::vec3 p_DampingEulerAngles;

    private:
        inline static constexpr const char* ConfigSection = "MoveToCamera";

        inline void InitPublicVariables() {
            p_DampingLinear = Config::GetOrSet<f32>(std::string(ConfigSection), "LinearDamping", 10.0f);
            p_DampingEulerAngles.x = Config::GetOrSet<f32>(std::string(ConfigSection), "DampingX", 5.0f);
            p_DampingEulerAngles.y = Config::GetOrSet<f32>(std::string(ConfigSection), "DampingY", 5.0f);
            p_DampingEulerAngles.z = Config::GetOrSet<f32>(std::string(ConfigSection), "DampingZ", 5.0f);
        }

        inline static f32 ClipAngle(f32 a) {
            if (a < -180.0f)
                return a + 360.0f;
            if (a > 180.0f)
                return a - 360.0f;
            return a;
        }

        inline static glm::vec3 ClipAngles(const glm::vec3& angles) {
            return glm::vec3(std::fmod(angles.x, 360.0f), std::fmod(angles.y, 360.0f), std::fmod(angles.z, 360.0f));
        }

        inline static glm::vec3 AngleDelta(const glm::vec3& current, const glm::vec3& desired) {
            const glm::vec3 d = ClipAngles(current) - ClipAngles(desired);
            return glm::vec3(ClipAngle(d.x), ClipAngle(d.y), ClipAngle(d.z));
        }

        glm::vec3 m_PositionCurrent = glm::vec3(0.0f);
        glm::vec3 m_PositionDesired = glm::vec3(0.0f);

        // pitch, yaw, roll
        glm::vec3 m_AnglesCurrent = glm::vec3(0.0f);
        glm::vec3 m_AnglesDesired = glm::vec3(0.0f);

        glm::mat4 m_CurrentTransform = glm::mat4(0.0f);
    };
} // namespace Axle
