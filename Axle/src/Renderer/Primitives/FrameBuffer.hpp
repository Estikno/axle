#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Textures/Texture.hpp"

namespace Axle {
    class AXLE_API FrameBuffer : public RefCounted {
    public:
        FrameBuffer() = default;
        FrameBuffer(const Ref<Texture2D>& color, bool isDepthNeeded, bool isStencilNeeded);

        ~FrameBuffer() override;

        FrameBuffer(FrameBuffer&& other) noexcept;
        FrameBuffer& operator=(FrameBuffer&& other) noexcept;

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        inline u32 GetID() const {
            return m_ID;
        }

        inline u32 GetRenderBufferID() const {
            return m_RenderBufferID;
        }

        void Bind() const;
        void UnBind() const;
        static void BindDefault();

    private:
        void Reset();

        u32 m_ID = 0, m_RenderBufferID = 0;

        Ref<Texture2D> m_Color;
    };
} // namespace Axle
