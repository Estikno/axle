#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Textures/Texture.hpp"

namespace Axle {
    /**
     * RAII wrapper of an OpenGL FrameBuffer
     *
     * All the functionality of this class is NOT THREAD SAFE and must only be accessed by the render thread.
     * */
    class AXLE_API FrameBuffer : public RefCounted {
    public:
        FrameBuffer() = default;

        /**
         * Creates a FrameBuffer
         *
         * @param color Texture2D that will be the color attachment. From the texture will the width and height be
         * implied
         * @param isDepthNeeded Does it need a depth buffer?
         * @param isStencilNeeded Does it need a stencil buffer?
         * */
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

        /**
         * Binds the FrameBuffer and also sets the render viewport based on the ouput texture
         * */
        void Bind() const;

        /**
         * Unbinds the FrameBuffer (binds the default one). Automcatically sets the viewport to full screen.
         * */
        void UnBind() const;

        /**
         * Binds the default FrameBuffer. Automcatically sets the viewport to full screen.
         * */
        static void BindDefault();

    private:
        void Reset();

        u32 m_ID = 0, m_RenderBufferID = 0;

        Ref<Texture2D> m_Color;
    };
} // namespace Axle
