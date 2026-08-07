#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Other/CustomTypes/Ref.hpp"

namespace Axle {
    /**
     * RAII wrapper of an OpenGL UniformBuffer
     *
     * All the functionality of this class is NOT THREAD SAFE and must only be accessed by the render thread.
     * */
    class UniformBuffer : public RefCounted {
    public:
        /// This constructor does nothing
        UniformBuffer() = default;

        /**
         * Creates a UniformBuffer
         *
         * @param size The size of the data to store
         * @param data Pointer to the data to store
         * */
        UniformBuffer(u32 size, const void* data);

        ~UniformBuffer() override;

        UniformBuffer(UniformBuffer&& other) noexcept;
        UniformBuffer& operator=(UniformBuffer&& other) noexcept;

        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        inline u32 GetID() const {
            return m_ID;
        }

        /**
         * Binds the UniformBuffer to the specified binding index
         *
         * @param bindingIndex Index to bind the buffer
         * */
        void Bind(u32 bindingIndex) const;

        /**
         * Updates the data in the buffer
         *
         * @param offset Offset into the buffer object where it should begin updating
         * @param size Size of the modified data
         * @param data Pointer to the updated data
         * */
        void UpdateData(u32 offset, u32 size, const void* data);

    private:
        void Reset();

        u32 m_ID = 0;
    };
} // namespace Axle
