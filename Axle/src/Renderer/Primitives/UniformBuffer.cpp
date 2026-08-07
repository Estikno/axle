#include "axpch.hpp"

#include <glad/gl.h>

#include "UniformBuffer.hpp"
#include "Renderer/GLDebug.hpp"

namespace Axle {
    UniformBuffer::UniformBuffer(u32 size, const void* data) {
        AX_GL_CALL(glCreateBuffers(1, &m_ID));
        AX_GL_CALL(glNamedBufferData(m_ID, size, data, GL_DYNAMIC_DRAW));
    }

    UniformBuffer::~UniformBuffer() {
        Reset();
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
        : m_ID(other.m_ID) {
        other.m_ID = 0;
    }

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
        if (this != &other) {
            Reset();

            m_ID = other.m_ID;
            other.m_ID = 0;
        }
        return *this;
    }

    void UniformBuffer::Bind(u32 bindingIndex) const {
        AX_GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, m_ID));
    }

    void UniformBuffer::UpdateData(u32 offset, u32 size, const void* data) {
        AX_GL_CALL(glNamedBufferSubData(m_ID, offset, size, data));
    }

    void UniformBuffer::Reset() {
        if (m_ID != 0) {
            AX_GL_CALL(glDeleteBuffers(1, &m_ID));
        }
    }
} // namespace Axle
