#include "axpch.hpp"

#include <glad/gl.h>

#include "Core/Types.hpp"
#include "Buffer.hpp"
#include "Renderer/GLDebug.hpp"

#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    // -----------------------------------------------------
    // Vertex Buffer
    // -----------------------------------------------------

    VertexBuffer::VertexBuffer(u32 size, f32* vertices) {
        TracyGpuZone("Create VertexBuffer");

        AX_GL_CALL(glCreateBuffers(1, &m_ID));
        AX_GL_CALL(glNamedBufferData(m_ID, size, vertices, GL_STATIC_DRAW));
    }

    VertexBuffer::~VertexBuffer() {
        Reset();
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
        : m_ID(other.m_ID) {
        other.m_ID = 0;
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
        if (this != &other) {
            Reset();

            m_ID = other.m_ID;
            other.m_ID = 0;
        }
        return *this;
    }

    void VertexBuffer::Reset() {
        if (m_ID != 0) {
            AX_GL_CALL(glDeleteBuffers(1, &m_ID));
        }
    }

    // -----------------------------------------------------
    // Index Buffer
    // -----------------------------------------------------

    IndexBuffer::IndexBuffer(u32 count, u32* indices)
        : m_Count(count) {
        TracyGpuZone("Create IndexBuffer");

        AX_GL_CALL(glCreateBuffers(1, &m_ID));
        AX_GL_CALL(glNamedBufferData(m_ID, sizeof(u32) * count, indices, GL_STATIC_DRAW));
    }

    IndexBuffer::~IndexBuffer() {
        Reset();
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : m_ID(other.m_ID),
          m_Count(other.m_Count) {
        other.m_ID = 0;
        other.m_Count = 0;
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
        if (this != &other) {
            Reset();

            m_ID = other.m_ID;
            m_Count = other.m_Count;

            other.m_ID = 0;
            other.m_Count = 0;
        }
        return *this;
    }

    void IndexBuffer::Reset() {
        if (m_ID != 0) {
            AX_GL_CALL(glDeleteBuffers(1, &m_ID));
        }
    }
} // namespace Axle
