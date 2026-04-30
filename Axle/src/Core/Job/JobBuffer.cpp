#include "axpch.hpp"

#include "JobBuffer.hpp"
#include "Other/CustomTypes/Expected.hpp"

namespace Axle {
    bool JobBuffer::TryPush(Job job) {
        // No lock — only owner thread calls this
        std::unique_lock<std::mutex> lock(m_Jobs[m_Head].m_Mutex, std::try_to_lock);

        // If we can't acquire the lock we simply return
        if (!lock.owns_lock())
            return false;

        // If the current pointing node has a valid job already then it means the queue is full and we can't append
        // another job
        if (m_Jobs[m_Head].m_Value.has_value())
            return false;

        m_Jobs[m_Head].m_Value = job;
        m_Head = (m_Head + 1) % m_Jobs.size();
        return true;
    }

    Expected<Job> JobBuffer::Pop() {
        // No need to lock the head because only the thread owner calls the this method
        u32 newHead = (m_Head == 0) ? m_Jobs.size() - 1 : m_Head - 1;

        // A worker thread poping a job has nothing else to do so the best option is to block until we can access
        // the node
        std::scoped_lock lock(m_Jobs[newHead].m_Mutex);

        if (!m_Jobs[newHead].m_Value.has_value())
            return Expected<Job>::FromException(std::logic_error("The job list is empty"));

        Job job = std::move(*m_Jobs[newHead].m_Value);
        m_Jobs[newHead].m_Value.reset();
        m_Head = newHead;
        return job;
    }

    Expected<Job> JobBuffer::TrySteal() {
        // We try to lock the mutex because thread trying to steal may have other buffers to check so waiting for the
        // lock is waiting time
        std::unique_lock<std::mutex> lock(m_TailMutex, std::try_to_lock);

        if (!lock.owns_lock())
            return Expected<Job>::FromException(
                std::runtime_error("Can't access because the tail is currently locked"));

        u32 oldTail = m_Tail;

        // Same as before
        std::unique_lock<std::mutex> nodeLock(m_Jobs[oldTail].m_Mutex, std::try_to_lock);
        if (!nodeLock.owns_lock())
            return Expected<Job>::FromException(std::runtime_error("Node blocked"));

        if (!m_Jobs[oldTail].m_Value.has_value())
            return Expected<Job>::FromException(std::logic_error("The job list is empty"));

        Job job = std::move(*m_Jobs[oldTail].m_Value);
        m_Jobs[oldTail].m_Value.reset();
        m_Tail = (oldTail + 1) % m_Jobs.size();
        return job;
    }

    Expected<Job> JobBuffer::Steal() {
        std::scoped_lock lock(m_TailMutex);
        u32 oldTail = m_Tail;

        std::scoped_lock node_lock(m_Jobs[oldTail].m_Mutex);

        if (!m_Jobs[oldTail].m_Value.has_value())
            return Expected<Job>::FromException(std::logic_error("The job list is empty"));

        Job job = std::move(*m_Jobs[oldTail].m_Value);
        m_Jobs[oldTail].m_Value.reset();
        m_Tail = (oldTail + 1) % m_Jobs.size();
        return job;
    }
} // namespace Axle
