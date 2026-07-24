#include "axpch.hpp"

#include <glad/gl.h>

#include "VertexArray.hpp"
#include "Renderer/GLDebug.hpp"
#include "Renderer/Primitives/Buffer.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Renderer/Shaders/Shader.hpp"

#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    VertexArray::VertexArray() {
        AX_GL_CALL(glCreateVertexArrays(1, &m_ID));
    }

    VertexArray::~VertexArray() {
        Reset();
    }

    VertexArray::VertexArray(VertexArray&& other) noexcept
        : m_ID(other.m_ID),
          m_AttribIndex(other.m_AttribIndex),
          m_VertexBuffers(std::move(other.m_VertexBuffers)),
          m_IndexBuffer(std::move(other.m_IndexBuffer)) {
        other.m_ID = 0;
    }

    VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
        if (this != &other) {
            Reset();

            m_ID = other.m_ID;
            m_AttribIndex = other.m_AttribIndex;
            m_VertexBuffers = std::move(other.m_VertexBuffers);
            m_IndexBuffer = std::move(other.m_IndexBuffer);

            other.m_ID = 0;
        }
        return *this;
    }

    void VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) {
        TracyGpuZone("Add VertexBuffer to VertexArray");

        AX_ASSERT(
            vertexBuffer->GetLayout().GetElements().size() > 0, LogChannel::Renderer, "VertexBuffer has no layout.");

        u32 bindingIndex = m_VertexBuffers.size();
        const BufferLayout& layout = vertexBuffer->GetLayout();

        AX_GL_CALL(glVertexArrayVertexBuffer(m_ID, bindingIndex, vertexBuffer->GetID(), 0, layout.GetStride()));

        for (const BufferElement& element : layout) {
            AX_GL_CALL(glEnableVertexArrayAttrib(m_ID, m_AttribIndex));
            AX_GL_CALL(glVertexArrayAttribBinding(m_ID, m_AttribIndex, bindingIndex));
            AX_GL_CALL(glVertexArrayAttribFormat(m_ID,
                                                 m_AttribIndex,
                                                 element.GetComponentCount(),
                                                 ShaderDataTypeToOpenGLBaseType(element.Type),
                                                 element.Normalized ? GL_TRUE : GL_FALSE,
                                                 element.Offset));

            m_AttribIndex++;
        }

        m_VertexBuffers.push_back(vertexBuffer);
    }

    void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) {
        TracyGpuZone("Add IndexBuffer to VertexArray");

        AX_GL_CALL(glVertexArrayElementBuffer(m_ID, indexBuffer->GetID()));

        m_IndexBuffer = indexBuffer;
    }

    void VertexArray::Bind() const {
        AX_GL_CALL(glBindVertexArray(m_ID));
    }
    void VertexArray::Unbind() const {
        AX_GL_CALL(glBindVertexArray(0));
    }

    void VertexArray::Reset() {
        if (m_ID != 0) {
            AX_GL_CALL(glDeleteVertexArrays(1, &m_ID));
        }
    }

} // namespace Axle
