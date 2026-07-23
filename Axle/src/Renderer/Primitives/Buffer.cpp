#include "axpch.hpp"

#include <glad/gl.h>

#include "Core/Types.hpp"
#include "Buffer.hpp"

namespace Axle {
    // -----------------------------------------------------
    // Vertex Buffer
    // -----------------------------------------------------

    VertexBuffer::VertexBuffer(u32 size, f32* vertices) {
        glCreateBuffers(1, &m_ID);
        glNamedBufferData(m_ID, size, vertices, GL_STATIC_DRAW);
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &m_ID);
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
        : m_ID(other.m_ID) {
        other.m_ID = 0;
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
        if (this != &other) {
            glDeleteBuffers(1, &m_ID);

            m_ID = other.m_ID;
            other.m_ID = 0;
        }
        return *this;
    }

    // -----------------------------------------------------
    // Index Buffer
    // -----------------------------------------------------

    IndexBuffer::IndexBuffer(u32 count, u32* indices)
        : m_Count(count) {
        glCreateBuffers(1, &m_ID);
        glNamedBufferData(m_ID, sizeof(u32) * count, indices, GL_STATIC_DRAW);
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &m_ID);
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : m_ID(other.m_ID),
          m_Count(other.m_Count) {
        other.m_ID = 0;
        other.m_Count = 0;
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
        if (this != &other) {
            glDeleteBuffers(1, &m_ID);

            m_ID = other.m_ID;
            m_Count = other.m_Count;

            other.m_ID = 0;
            other.m_Count = 0;
        }
        return *this;
    }
} // namespace Axle
