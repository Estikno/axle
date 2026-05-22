#pragma once

#include "axpch.hpp"

#include "ECS.hpp"
#include "Core/Types.hpp"
#include "Core/Core.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

namespace Axle {
    class AXLE_TEST_API Systems {
    public:
        /**
         * Excecutes all the systems added to the systems manager.
         *
         * The RefreshBatches method must be called after adding new batches and before this one, otherwise the behavior
         * is undifined on non-debug builds.
         * */
        void Update();

        /**
         * Refreshes system collitions and how they are excecuted. Has to be used after adding a new system.
         *
         * This precomputation allows updates to happen faster.
         * */
        void RefreshBatches();

        /**
         * Registers a system (lambda or function) along with its component
         * dependencies. At registration time we record which components are read
         * (const) and which are written (non-const). The actual batch schedule is
         * not computed here — call RefreshBatches() once after all systems are
         * added.
         *
         * This method assumes all used Components are registered correctly. If not the behavior is undifined.
         *
         * Usage:
         *   // Writer — receives a live reference, can mutate
         *   systems.Add<Velocity>([](Velocity& vel) { vel.vx += 1.0f; });
         *
         *   // Reader — receives a copy of the previous frame's value
         *   systems.Add<Position, const Velocity>([](Position& pos, const Velocity& vel) {
         *       pos.x += vel.vx;
         *   });
         *
         *   // With EntityID as first parameter (optional)
         *   systems.Add<Position, const Velocity>([](EntityID id, Position& pos, const Velocity& vel) { ... });
         * */
        template <typename... Components, typename Func>
        void Add(Func&& func) {
            // We create a wrapper around each function so that we can call it with the ECS reference
            // This simplifies the storing process.
            SystemEntry entry;

            // Populate reader/writer sets
            (([&]() {
                 ComponentType type = ECS::GetInstance().GetComponentType<CleanComponent<Components>>();
                 if constexpr (IsReadOnly<Components>)
                     entry.readers.insert(type);
                 else
                     entry.writers.insert(type);
             })(),
             ...);

            // Detects if the lambda has an EntityID parameter or not
            if constexpr (std::is_invocable_v<Func, EntityID, Components&...>) {
                entry.func = [func = std::forward<Func>(func)](FrameData& frame) {
                    // Get the entity list for this specific combination of components.
                    // All entities that have every component in the pack are included.
                    std::vector<EntityID>& entityList = frame.GetEntities<CleanComponent<Components>...>();

                    for (EntityID id : entityList) {
                        // For each entity, retrieve either a snapshot value (const)
                        // or a live reference (non-const) depending on the pack.
                        // std::apply unpacks the tuple into the lambda's arguments.
                        auto refs = std::tie(frame.Get<Components>(id)...);
                        std::apply([&](Components&... comps) { func(id, comps...); }, refs);
                    }
                };
            } else {
                // The case: lambda without EntityID
                entry.func = [func = std::forward<Func>(func)](FrameData& frame) {
                    auto& entityList = frame.GetEntities<CleanComponent<Components>...>();

                    for (EntityID id : entityList) {
                        auto refs = std::tie(frame.Get<Components>(id)...);
                        std::apply(func, refs);
                    }
                };
            }

            m_Systems.emplace_back(std::move(entry));
            m_BatchesDirty = true;
        }

    private:
        // Helper to strip const and reference from a type for storage/lookup purposes.
        // e.g. const Velocity& -> Velocity
        template <typename T>
        using CleanComponent = std::decay_t<std::remove_const_t<T>>;

        // Detects whether a component type is marked const by the user.
        // const Velocity  -> true  (read-only)
        // Velocity        -> false (writable)
        template <typename T>
        inline static constexpr bool IsReadOnly = std::is_const_v<std::remove_reference_t<T>>;

        /**
         * Holds all component data needed for one frame. Built once at the start
         * of Update() by querying the ECS, then passed to every system.
         * */
        struct FrameData {
        public:
            // Snapshots: ComponentType -> flat buffer of copied component values
            // Index into the buffer using the entity's position in the entity list
            std::unordered_map<ComponentType, std::vector<std::byte>> m_Snapshots;

            // Live pointers: ComponentType -> pointer per entity into ECS storage
            std::unordered_map<ComponentType, std::vector<void*>> m_LivePtrs;

            // Entity lists: sorted component set -> list of matching EntityIDs
            // The key is a sorted vector of ComponentTypes (the combination).
            std::map<std::vector<ComponentType>, std::vector<EntityID>> m_EntityLists;

            // Maps EntityID to its index in the entity list for a given combination,
            // so Get<T>(id) can find the right slot in O(1)
            std::map<std::vector<ComponentType>, std::unordered_map<EntityID, size_t>> m_EntityIndex;

            FrameData(const std::unordered_set<ComponentType>& readers,
                      const std::unordered_set<ComponentType>& writers,
                      const std::vector<std::vector<ComponentType>>& componentSets) {
                ECS& ecs = ECS::GetInstance();

                // For each unique component combination a system needs,
                // query the ECS once and store the results
                for (auto& sortedSet : componentSets) {
                    // Ask the ECS for all entities that have every component
                    // in this set. This is the same thing View does internally.
                    std::vector<EntityID> entities = QueryEntities(ecs, sortedSet);

                    // Build the reverse index: EntityID -> position in the list
                    std::unordered_map<EntityID, size_t> index;
                    index.reserve(entities.size());
                    for (size_t i = 0; i < entities.size(); ++i)
                        index[entities[i]] = i;

                    m_EntityLists[sortedSet] = std::move(entities);
                    m_EntityIndex[sortedSet] = std::move(index);
                }

                // Copy snapshot data for every read-only component type
                for (ComponentType type : readers) {
                    // We copy the raw component bytes out of the SparseSet
                    // so readers always see the start-of-frame values
                    m_Snapshots[type] = ecs.CopyComponentBuffer(type);
                }

                // Store raw pointers for writable components (no copy)
                for (ComponentType type : writers) {
                    m_LivePtrs[type] = ecs.GetComponentPtrs(type);
                }
            }

            // Returns the entity list for a given component combination
            template <typename... Ts>
            std::vector<EntityID>& GetEntities() {
                auto key = MakeKey<Ts...>();
                return m_EntityLists.at(key);
            }

            // Returns either a snapshot value (if const T) or a live reference
            // (if non-const T) for the given entity
            template <typename T>
            T& Get(EntityID id) {
                ComponentType type = ECS::GetInstance().GetComponentType<CleanComponent<T>>();

                if constexpr (IsReadOnly<T>) {
                    // Read from snapshot buffer — safe to access from any thread
                    // since no system writes to snapshots
                    auto& buffer = m_Snapshots.at(type);
                    size_t offset = id * sizeof(CleanComponent<T>);
                    return *reinterpret_cast<CleanComponent<T>*>(buffer.data() + offset);
                } else {
                    // Return the live pointer — safe because writer systems
                    // are guaranteed by RefreshBatches() to never share a
                    // writable component with another concurrent system
                    return *reinterpret_cast<T*>(m_LivePtrs.at(type)[id]);
                }
            }

        private:
            template <typename... Ts>
            static std::vector<ComponentType> MakeKey() {
                // Sort the component types so the same set always produces the
                // same key regardless of the order in the template pack
                std::vector<ComponentType> key = {ECS::GetInstance().GetComponentType<Ts>()...};
                std::sort(key.begin(), key.end());
                return key;
            }
        };

        struct SystemEntry {
            std::function<void(FrameData&)> func;
            std::unordered_set<ComponentType> readers;
            std::unordered_set<ComponentType> writers;
        };

        static bool HasOverlap(const std::unordered_set<ComponentType>& a, const std::unordered_set<ComponentType>& b) {
            const auto& [smaller, larger] = a.size() <= b.size() ? std::tie(a, b) : std::tie(b, a);
            for (const ComponentType& type : smaller)
                if (larger.count(type))
                    return true;
            return false;
        }


        /// A vector to store all the system wrappers to the real functions
        std::vector<SystemEntry> m_Systems;

        // Precomputed execution schedule: each inner vector is one batch of
        // indices into m_Systems that can run in parallel
        std::vector<std::vector<size_t>> m_Batches;

        // Precomputed at RefreshBatches() time: the unique sorted component
        // combinations needed across all systems, used to build FrameData
        std::vector<std::vector<ComponentType>> m_ComponentSets;

        bool m_BatchesDirty = false;
    };
} // namespace Axle
