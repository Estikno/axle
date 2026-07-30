#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Other/CustomTypes/Ref.hpp"

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

    /**
     * Layout used in vertex buffer for easily setting attributes and all of their information.
     * */
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
    class VertexBuffer : public RefCounted {
    public:
        /// This constructor does nothing
        VertexBuffer() = default;

        /**
         * Create a vertex buffer
         *
         * @param size Size of the attribute data (in bytes)
         * @param vertices Pointer to the attribute data
         * */
        VertexBuffer(u32 size, f32* vertices);
        ~VertexBuffer() override;

        VertexBuffer(VertexBuffer&& other) noexcept;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept;

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        /**
         * Sets how the data shall be interpreted.
         *
         * @param layout Data layout
         * */
        inline void SetLayout(const BufferLayout& layout) {
            m_Layout = layout;
        }

        /**
         * Returns the layout of the vertex data (if any)
         * */
        inline const BufferLayout& GetLayout() const {
            return m_Layout;
        }

        /**
         * Returns the underlying id provided by OpenGL. All OpenGL related stuff will automatically be handled by this
         * class. Usage of the ID in OpenGL calls is not recommended.
         * */
        inline u32 GetID() const {
            return m_ID;
        }

    private:
        /**
         * Deallocated all GPU memory (if any was allocated in the first place)
         * */
        void Reset();

        u32 m_ID = 0;
        BufferLayout m_Layout;
    };

    /**
     * RAII wrapper of an OpenGL index/element buffer
     * */
    class ElementBuffer : public RefCounted {
    public:
        /// This constructor does nothing
        ElementBuffer() = default;

        /**
         * Create an element buffer
         *
         * @param count How many indices does it have.
         * @param indices Pointer to the indices array
         * */
        ElementBuffer(u32 count, u32* indices);
        ~ElementBuffer() override;

        ElementBuffer(ElementBuffer&& other) noexcept;
        ElementBuffer& operator=(ElementBuffer&& other) noexcept;

        ElementBuffer(const ElementBuffer&) = delete;
        ElementBuffer& operator=(const ElementBuffer&) = delete;

        /**
         * Returns the underlying id provided by OpenGL. All OpenGL related stuff will automatically be handled by this
         * class. Usage of the ID in OpenGL calls is not recommended.
         * */
        inline u32 GetID() const {
            return m_ID;
        }

        /**
         * Gets the count of the element buffer
         * */
        inline u32 GetCount() const {
            return m_Count;
        }

    private:
        /**
         * Deallocated all GPU memory (if any was allocated in the first place)
         * */
        void Reset();

        u32 m_ID = 0;
        u32 m_Count = 0;
    };
} // namespace Axle
