#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Renderer/Shaders/Shader.hpp"

namespace Axle {
    struct BufferElement {
        std::string Name;
        ShaderDataType Type = ShaderDataType::None;
        u32 Size = 0;
        u32 Offset = 0;
        bool Normalized = false;

        BufferElement() = default;
        BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
            : Name(name),
              Type(type),
              Size(ShaderDataTypeSize(type)),
              Offset(0),
              Normalized(normalized) {}

        u32 GetComponentCount() const {
            switch (Type) {
                case ShaderDataType::Float:
                    return 1;
                case ShaderDataType::Vec2:
                    return 2;
                case ShaderDataType::Vec3:
                    return 3;
                case ShaderDataType::Vec4:
                    return 4;
                case ShaderDataType::Mat3:
                    return 3 * 3;
                case ShaderDataType::Mat4:
                    return 4 * 4;
                case ShaderDataType::Int:
                    return 1;
                case ShaderDataType::Int2:
                    return 2;
                case ShaderDataType::Int3:
                    return 3;
                case ShaderDataType::Int4:
                    return 4;
                case ShaderDataType::Bool:
                    return 1;
            }

            AX_PANIC(LogChannel::Renderer, "Unknown ShaderDataType!");
        }
    };

    class BufferLayout {
    public:
        BufferLayout() = default;
        BufferLayout(const std::initializer_list<BufferElement>& elements)
            : m_Elements(elements) {
            CalculateOffsetsAndStride();
        }

        inline u32 GetStride() const {
            return m_Stride;
        }
        inline const std::vector<BufferElement>& GetElements() const {
            return m_Elements;
        }

        std::vector<BufferElement>::iterator begin() {
            return m_Elements.begin();
        }
        std::vector<BufferElement>::iterator end() {
            return m_Elements.end();
        }

        std::vector<BufferElement>::const_iterator begin() const {
            return m_Elements.cbegin();
        }
        std::vector<BufferElement>::const_iterator end() const {
            return m_Elements.cend();
        }

    private:
        void CalculateOffsetsAndStride() {
            u32 offset = 0;
            m_Stride = 0;

            for (BufferElement& element : m_Elements) {
                element.Offset = offset;
                offset += element.Size;
                m_Stride += element.Size;
            }
        }

        std::vector<BufferElement> m_Elements;
        u32 m_Stride = 0;
    };

    /**
     * RAII wrapper of an OpenGL vertex buffer
     * */
    class VertexBuffer {
    public:
        VertexBuffer() = default;
        VertexBuffer(u32 size, f32* vertices);
        ~VertexBuffer();

        VertexBuffer(VertexBuffer&& other) noexcept;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept;

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        inline void SetLayout(const BufferLayout& layout) {
            m_Layout = layout;
        }
        inline const BufferLayout& GetLayout() const {
            return m_Layout;
        }

        inline u32 GetID() const {
            return m_ID;
        }

    private:
        u32 m_ID = 0;
        BufferLayout m_Layout;
    };

    /**
     * RAII wrapper of an OpenGL index/element buffer
     * */
    class IndexBuffer {
    public:
        IndexBuffer() = default;
        IndexBuffer(u32 count, u32* indices);
        ~IndexBuffer();

        IndexBuffer(IndexBuffer&& other) noexcept;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept;

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        inline u32 GetID() const {
            return m_ID;
        }

        inline u32 GetCount() const {
            return m_Count;
        }

    private:
        u32 m_ID = 0;
        u32 m_Count = 0;
    };
} // namespace Axle
