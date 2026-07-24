#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Renderer/Primitives/Buffer.hpp"
#include "Other/CustomTypes/Ref.hpp"

namespace Axle {
    /**
     * RAII wrapper of an OpenGL vertex array
     * */
    class VertexArray : RefCounted {
    public:
        VertexArray();
        ~VertexArray() override;

        VertexArray(VertexArray&& other) noexcept;
        VertexArray& operator=(VertexArray&& other) noexcept;

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
        void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

        void Bind() const;
        void Unbind() const;

        inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const {
            return m_VertexBuffers;
        }

        inline const Ref<IndexBuffer>& GetIndexBuffer() const {
            return m_IndexBuffer;
        }

        inline u32 GetID() const {
            return m_ID;
        }

    private:
        void Reset();

        u32 m_ID = 0;
        u32 m_AttribIndex = 0;

        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer> m_IndexBuffer;
    };
} // namespace Axle
