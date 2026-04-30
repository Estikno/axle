#pragma once

#include "axpch.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/Expected.hpp"

namespace Axle {
    /**
     * Fairly basic RingBuffer used by the JobSystem
     *
     * Important: This class is not thread safe if used incorrectly so read carefuly the methods comments
     * */
    class JobBuffer {
    public:
        JobBuffer(u32 size)
            : m_Jobs(size),
              m_Head(0),
              m_Tail(0) {}

        /**
         * Tries to push a job to the list.
         *
         * If the mutex is blocked or the list itself is empty it fails.
         * Even if the pushed failed the job system can progress.
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
         * If the mutex is blocked or the list itself is empty it fails.
         *
         * @returns An Expected value, it's valid if there was a job to steal and the value is that poped job, otherwise
         * it's invalid.
         * */
        Expected<Job> TrySteal();

#ifdef AXLE_TESTING
        u32 GetSize() {
            return m_Jobs.size();
        }
#endif // AXLE_TESTING

    private:
        // Simple job wrapper so that we can have QOL stuff
        struct Node {
            Node() = default;
            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;

            std::mutex m_Mutex;
            std::optional<Job> m_Value;
        };

        // The head points to the side were jobs are pushed and poped
        u32 m_Head;

        // The head points to the other end of the buffer, where jobs are stolen
        u32 m_Tail;
        std::mutex m_TailMutex;

        // We don't need to change the size of the buffer so a simple vector is enough
        std::vector<Node> m_Jobs;
    };
} // namespace Axle
