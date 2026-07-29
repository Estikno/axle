#pragma once

#include "axpch.hpp"

#include "Core/Logger/Log.hpp"
#include "Core/Types.hpp"
#include "Renderer/Primitives/Buffer.hpp"
#include "Other/CustomTypes/Ref.hpp"

namespace Axle {
    /**
     * RAII wrapper of an OpenGL vertex array
     * */
    class VertexArray : public RefCounted {
    public:
        VertexArray();
        ~VertexArray() override;

        VertexArray(VertexArray&& other) noexcept;
        VertexArray& operator=(VertexArray&& other) noexcept;

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
        void SetIndexBuffer(const Ref<ElementBuffer>& indexBuffer);

        void Bind() const;
        void Unbind() const;

        inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const {
            return m_VertexBuffers;
        }

        inline const Ref<ElementBuffer>& GetElementBuffer() const {
            return m_ElementBuffer;
        }

        inline u32 GetID() const {
            return m_ID;
        }

    private:
        void Reset();

        u32 m_ID = 0;
        u32 m_AttribIndex = 0;

        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<ElementBuffer> m_ElementBuffer;
    };
} // namespace Axle
