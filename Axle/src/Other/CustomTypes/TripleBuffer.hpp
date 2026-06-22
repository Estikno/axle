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
            return m_Buffers[m_BackIdx];
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

            return {m_Buffers[m_FrontIdx], updated};
        }

    private:
        struct State {
            u8 idx;
            bool hasUpdate;
        };
        static_assert(std::atomic<State>::is_always_lock_free, "The TripleBuffer is not always lock-free");

        T m_Buffers[3];
        u8 m_FrontIdx{2};
        std::atomic<State> m_Spare{{1, false}};
        u8 m_BackIdx{2};
    };
} // namespace Axle
