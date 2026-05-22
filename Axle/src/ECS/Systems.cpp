#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"
#include "Systems.hpp"
#include "Core/Job/JobSystem.hpp"

namespace Axle {
    void Systems::Update() {
        AX_ASSERT(!m_BatchesDirty, "RefreshBatches() must be called after adding new systems");

        // --- Build the frame snapshot ---
        // Collect all component types needed across all systems
        std::unordered_set<ComponentType> allReaders;
        std::unordered_set<ComponentType> allWriters;
        for (auto& system : m_Systems) {
            allReaders.insert(system.readers.begin(), system.readers.end());
            allWriters.insert(system.writers.begin(), system.writers.end());
        }

        // FrameData queries the ECS once and holds:
        //   - snapshots (copies) for all read-only component types
        //   - live references for all writable component types
        //   - entity lists for each unique component combination needed
        FrameData frame(allReaders, allWriters, m_ComponentSets);

        // --- Dispatch batches ---
        // Same as before: systems within a batch run in parallel,
        // batches themselves run sequentially. But now each system
        // receives the pre-built FrameData instead of querying the ECS.
        for (std::vector<size_t>& batch : m_Batches) {
            std::vector<JobFuture<void>> futures;
            futures.reserve(batch.size());

            for (size_t idx : batch) {
                futures.push_back(
                    JobSystem::GetInstance().Submit<void>([this, &frame, idx]() { m_Systems[idx].func(frame); }));
            }

            for (auto& future : futures)
                future.Wait();
        }
    }

    void Systems::RefreshBatches() {
        m_Batches.clear();

        // Track which systems have already been placed into a batch
        std::vector<bool> scheduled(m_Systems.size(), false);

        for (size_t i = 0; i < m_Systems.size(); ++i) {
            if (scheduled[i])
                continue;

            // Start a new batch with system i as its first member
            std::vector<size_t> batch = {i};
            scheduled[i] = true;

            // The combined reader/writer sets of everything in this batch.
            // Used to check incoming systems for conflicts.
            std::unordered_set<ComponentType> batchWriters = m_Systems[i].writers;
            std::unordered_set<ComponentType> batchReaders = m_Systems[i].readers;

            // Try to add every later system into this same batch
            for (size_t j = i + 1; j < m_Systems.size(); ++j) {
                if (scheduled[j])
                    continue;

                // A conflict exists if the candidate system writes something
                // the batch already reads/writes, or reads something the
                // batch writes.
                bool conflict = HasOverlap(m_Systems[j].writers, batchWriters) || // write-write
                                HasOverlap(m_Systems[j].writers, batchReaders) || // candidate writes, batch reads
                                HasOverlap(m_Systems[j].readers, batchWriters);   // candidate reads, batch writes

                if (!conflict) {
                    batch.push_back(j);
                    scheduled[j] = true;

                    // Merge the new system's dependencies into the batch sets
                    // so future candidates are checked against the full batch
                    batchWriters.insert(m_Systems[j].writers.begin(), m_Systems[j].writers.end());
                    batchReaders.insert(m_Systems[j].readers.begin(), m_Systems[j].readers.end());
                }
            }

            m_Batches.push_back(std::move(batch));
        }

        m_BatchesDirty = false;
    }
} // namespace Axle
