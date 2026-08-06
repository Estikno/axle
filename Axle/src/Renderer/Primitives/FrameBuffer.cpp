#include "axpch.hpp"

#include <glad/gl.h>

#include "FrameBuffer.hpp"
#include "Renderer/GLDebug.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    FrameBuffer::FrameBuffer(const Ref<Texture2D>& color, bool isDepthNeeded, bool isStencilNeeded)
        : m_Color(color) {
        AX_GL_CALL(glCreateFramebuffers(1, &m_ID));

        AX_GL_CALL(glNamedFramebufferTexture(m_ID, GL_COLOR_ATTACHMENT0, m_Color->GetID(), 0));

        // Attach renderbuffer only if necessary
        if (isDepthNeeded || isStencilNeeded) {
            AX_GL_CALL(glCreateRenderbuffers(1, &m_RenderBufferID));
            AX_GL_CALL(glNamedRenderbufferStorage(
                m_RenderBufferID, GL_DEPTH24_STENCIL8, m_Color->GetWidth(), m_Color->GetHeight()));
            AX_GL_CALL(
                glNamedFramebufferRenderbuffer(m_ID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBufferID));
        }

        glNamedFramebufferDrawBuffer(m_ID, GL_COLOR_ATTACHMENT0);

#ifdef AX_DEBUG
        // Check errors
        if (glCheckNamedFramebufferStatus(m_ID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            AX_PANIC(LogChannel::Renderer, "Render buffer is not complete");
        }
#endif // AX_DEBUG
    }

    FrameBuffer::~FrameBuffer() {
        Reset();
    }

    FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
        : m_ID(other.m_ID),
          m_RenderBufferID(other.m_RenderBufferID),
          m_Color(std::move(other.m_Color)) {
        other.m_ID = 0;
        other.m_RenderBufferID = 0;
    }

    FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
        if (this != &other) {
            Reset();

            m_ID = other.m_ID;
            m_RenderBufferID = other.m_RenderBufferID;
            m_Color = std::move(other.m_Color);

            other.m_ID = 0;
            other.m_RenderBufferID = 0;
        }
        return *this;
    }

    void FrameBuffer::Bind() const {
        AX_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_ID));
        RenderCommand::SetViewport(0, 0, m_Color->GetWidth(), m_Color->GetHeight());
    }

    void FrameBuffer::UnBind() const {
        BindDefault();
    }

    void FrameBuffer::BindDefault() {
        AX_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void FrameBuffer::Reset() {
        if (m_ID != 0) {
            AX_GL_CALL(glDeleteFramebuffers(1, &m_ID));
        }
        if (m_RenderBufferID != 0) {
            AX_GL_CALL(glDeleteRenderbuffers(1, &m_RenderBufferID));
        }
    }
} // namespace Axle
