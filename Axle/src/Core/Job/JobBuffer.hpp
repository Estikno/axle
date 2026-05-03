#pragma once

#include "axpch.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/Expected.hpp"

namespace Axle {
    /**
     * Fairly basic RingBuffer used by the JobSystem. The size must be a power of 2.
     *
     * We leave one slot permanently unused to distinguish “full” from “empty".
     * Important: This class is not thread safe if used incorrectly so read carefuly the methods comments
     * */
    template <std::size_t N>
    class JobBuffer {
    public:
        JobBuffer()
            : m_Head(0),
              m_Tail(0),
              m_TailCached(0),
              m_StealSemaphore(1) {}

        /**
         * Tries to push a job to the list. Returns without blocking if fails.
         *
         * Important: Only the "owner" thread of the buffer may push jobs.
         *
         * @param job The Job to be pushed
         *
         * @returns true if it was able to push the job, false otherwise
         * */
        bool TryPush(Job job);

        /**
         * Pops a Job from the list
         *
         * Important: Only the "owner" thread of the buffer may pop jobs.
         *
         * @returns An Expected value, it's valid if there was a job to pop and the value is that poped job, otherwise
         * it's invalid.
         * */
        Expected<Job> Pop();

        /**
         * Tries to steal a job from the list.
         *
         * If it can't steal it will return without blocking
         *
         * @returns An Expected value, it's valid if there was a job to steal and the value is that poped job, otherwise
         * it's invalid.
         * */
        Expected<Job> TrySteal();

        /**
         * It acts like TrySteal but this function will block until it can continue
         *
         * This method should not be the default one, it is highly recommended to use TrySteal unless it's strictly
         * necessary to block.
         *
         * @returns An Expected value, it's valid if there was a job to steal and the value is that poped job, otherwise
         * it's invalid.
         * */
        Expected<Job> Steal();

#ifdef AXLE_TESTING
        u32 GetSize() {
            return m_Jobs.size();
        }
#endif // AXLE_TESTING

    private:
        /**
         * Assumes that the semaphore has been acquired and proceeds to steal if it can.
         * This is obiosly thread unsafe if the semaphore hasn't been acquired.
         *
         * @returns An Expected value, it's valid if there was a job to steal and the value is that poped job, otherwise
         * it's invalid.
         * */
        Expected<Job> StealUnsafe();

        // The head points to the side were jobs are pushed and poped
        alignas(std::hardware_destructive_interference_size) std::atomic<u32> m_Head;
        // The head points to the other end of the buffer, where jobs are stolen
        alignas(std::hardware_destructive_interference_size) std::atomic<u32> m_Tail;

        // Cached values
        alignas(std::hardware_destructive_interference_size) u32 m_TailCached;

        std::array<std::optional<Job>, N> m_Jobs;
        static_assert((N & (N - 1)) == 0, "Size of the JobBuffer must be a power of 2");

        // Semaphore that ensures only one thread steals at a time
        std::binary_semaphore m_StealSemaphore;
    };


    template <std::size_t N>
    bool JobBuffer<N>::TryPush(Job job) {
        const u32 head = m_Head.load(std::memory_order_relaxed);
        u32 nextHead = (head + 1) & (N - 1);

        // The RingBuffer is full
        if (nextHead == m_TailCached) [[unlikely]] {
            m_TailCached = m_Tail.load(std::memory_order_acquire);
            if (nextHead == m_TailCached)
                return false;
        }

        m_Jobs[head] = std::move(job);
        m_Head.store(nextHead, std::memory_order_release);
        return true;
    }

    template <std::size_t N>
    Expected<Job> JobBuffer<N>::Pop() {
        const u32 head = m_Head.load(std::memory_order_relaxed);
        // There might be a problem if we use the cached value as we basically advance in
        // reverse, so for now we load the real tail.
        u32 tail = m_Tail.load(std::memory_order_acquire);

        u32 size = (head - tail) & (N - 1);
        if (size == 0) [[unlikely]] {
            return Expected<Job>::FromException(std::logic_error("The job list is empty"));
        }

        u32 nextHead = (head - 1) & (N - 1);

        // CRITICAL: The queue has exactly 1 item. We might collide with a Stealer.
        if (size == 1) {
            // Grab the semaphore to lock out any thieves
            m_StealSemaphore.acquire();

            // The thief might have stolen the last job so we need to re-read the tail
            tail = m_Tail.load(std::memory_order_acquire);
            size = (head - tail) & (N - 1);

            // The thief stole the last job, the buffer is now empty
            if (size == 0) {
                m_StealSemaphore.release();
                return Expected<Job>::FromException(std::logic_error("The job list is empty"));
            }

            // We secured the last job
            Job job = std::move(*m_Jobs[nextHead]);
            m_Jobs[nextHead].reset();
            m_Head.store(nextHead, std::memory_order_release);

            m_StealSemaphore.release();
            return job;
        }

        AX_ASSERT(m_Jobs[nextHead].has_value(), "Trying to dereference an optional with no value inside.");
        // There is more than one job, so no locks needed
        Job job = std::move(*m_Jobs[nextHead]);
        m_Jobs[nextHead].reset();
        m_Head.store(nextHead, std::memory_order_release);
        return job;
    }

    template <std::size_t N>
    Expected<Job> JobBuffer<N>::StealUnsafe() {
        const u32 tail = m_Tail.load(std::memory_order_relaxed);

        // The buffer is empty
        if (tail == m_Head.load(std::memory_order_acquire)) [[unlikely]] {
            m_StealSemaphore.release();
            return Expected<Job>::FromException(std::logic_error("The job list is empty"));
        }

        AX_ASSERT(m_Jobs[tail].has_value(), "Trying to dereference an optional with no value inside.");
        Job job = std::move(*m_Jobs[tail]);
        m_Jobs[tail].reset();

        u32 nextTail = (tail + 1) & (N - 1);
        m_Tail.store(nextTail, std::memory_order_release);

        m_StealSemaphore.release();
        return job;
    }

    template <std::size_t N>
    Expected<Job> JobBuffer<N>::TrySteal() {
        // There is already a thread trying to steal, return
        if (!m_StealSemaphore.try_acquire())
            return Expected<Job>::FromException(
                std::runtime_error("Can't access because the tail is currently locked"));

        return StealUnsafe();
    }

    template <std::size_t N>
    Expected<Job> JobBuffer<N>::Steal() {
        // Blocks until acquires the semaphore
        m_StealSemaphore.acquire();

        return StealUnsafe();
    }
} // namespace Axle
