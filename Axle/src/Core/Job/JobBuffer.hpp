#pragma once

#include "axpch.hpp"
#include "Core/Types.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include <stdexcept>

namespace Axle {
    // Fairly basic RingBuffer
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
         * @returns true if it was able to push the job, false otherwise
         * */
        bool TryPush(Job job) {
            // No lock — only owner thread calls this
            std::unique_lock<std::mutex> lock(m_Jobs[m_Head].m_Mutex, std::try_to_lock);

            // If we can't acquire the lock we simply return
            if (!lock.owns_lock())
                return false;

            // If the current pointing node has a valid job already then it means the queue is full and we can't append
            // another job
            if (m_Jobs[m_Head].value.has_value())
                return false;

            m_Jobs[m_Head].value = job;
            m_Head = (m_Head + 1) % m_Jobs.size();
            return true;
        }

        /**
         * Pops a Job from the list
         *
         * @returns An Expected value, it's valid if there was a job to pop and the value is that poped job, otherwise
         * it's invalid.
         * */
        Expected<Job> Pop() {
            u32 newHead = (m_Head == 0) ? m_Jobs.size() - 1 : m_Head - 1;

            // A worker thread poping a job has nothing else to do so the best option is to block until we can access
            // the node
            std::scoped_lock lock(m_Jobs[newHead].m_Mutex);

            if (!m_Jobs[newHead].value.has_value())
                return Expected<Job>::FromException(std::logic_error("The job list is empty"));

            Job job = std::move(*m_Jobs[newHead].value);
            m_Jobs[newHead].value.reset();
            m_Head = newHead;
            return job;
        }

        /**
         * Tries to steal a job from the list.
         *
         * If the mutex is blocked or the list itself is empty it fails.
         *
         * @returns An Expected value, it's valid if there was a job to steal and the value is that poped job, otherwise
         * it's invalid.
         * */
        Expected<Job> TrySteal() {
            std::unique_lock<std::mutex> lock(m_TailMutex, std::try_to_lock);

            if (!lock.owns_lock())
                return Expected<Job>::FromException(
                    std::runtime_error("Can't access because the tail is currently locked"));

            u32 oldTail = m_Tail;

            std::unique_lock<std::mutex> nodeLock(m_Jobs[oldTail].m_Mutex, std::try_to_lock);
            if (!nodeLock.owns_lock())
                return Expected<Job>::FromException(std::runtime_error("Node blocked"));

            if (!m_Jobs[oldTail].value.has_value())
                return Expected<Job>::FromException(std::logic_error("The job list is empty"));

            Job job = std::move(*m_Jobs[oldTail].value);
            m_Jobs[oldTail].value.reset();
            m_Tail = (oldTail + 1) % m_Jobs.size();
            return job;
        }

    private:
        struct Node {
            Node() = default;
            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;

            std::mutex m_Mutex;
            std::optional<Job> value;
        };

        u32 m_Head;

        u32 m_Tail;
        std::mutex m_TailMutex;

        std::vector<Node> m_Jobs;
    };
} // namespace Axle
