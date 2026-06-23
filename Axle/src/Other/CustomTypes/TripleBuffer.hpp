#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"

namespace Axle {
    /**
     * A fairly simple single producer/consumer lock-free triple buffer
     * */
    template <typename T>
    class TripleBuffer {
    public:
        TripleBuffer() = default;

        /**
         * Gets a reference to the back buffer.
         * To be used only by the producer.
         *
         * @returns A reference to the back buffer
         * */
        T& GetForWritter() {
            return m_Buffers[m_BackIdx].buf;
        }

        /**
         * Publishes the changes made to the back buffer by swapping it with the spare/middle one.
         * To be used only by the producer.
         * */
        void Publish() {
            State newSpare = {m_BackIdx, true};
            State prevSpare = m_Spare.exchange(newSpare, std::memory_order_acq_rel);
            m_BackIdx = prevSpare.idx;
        }

        /**
         * Gets a reference to the front buffer and additionally checks for updates and if it's the case swaps the front
         * and spare/middle buffer.
         *
         * @returns A pair that contains a reference to the front buffer (first) and a bool that tels if the buffer has
         * been updated (second)
         * */
        std::pair<T&, bool> GetForReader() {
            State currSpare = m_Spare.load(std::memory_order_relaxed);
            bool updated = currSpare.hasUpdate;

            if (currSpare.hasUpdate) {
                State newSpare = {m_FrontIdx, false};
                State prevSpare = m_Spare.exchange(newSpare, std::memory_order_acq_rel);
                m_FrontIdx = prevSpare.idx;
            }

            return {m_Buffers[m_FrontIdx].buf, updated};
        }

    private:
        static constexpr size_t KeepApartSZ = std::hardware_destructive_interference_size;

        struct State {
            u8 idx;
            bool hasUpdate;
        };

        struct alignas(KeepApartSZ) Buffer {
            T buf;
        };

        static_assert(std::atomic<State>::is_always_lock_free, "The TripleBuffer is not always lock-free");

        Buffer m_Buffers[3];
        alignas(KeepApartSZ) u8 m_FrontIdx{2};
        alignas(KeepApartSZ) std::atomic<State> m_Spare{{1, false}};
        alignas(KeepApartSZ) u8 m_BackIdx{2};
    };
} // namespace Axle
