#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"
#include "Other/Helpers/SparseSet.hpp"

namespace Axle {
    class Entities {
    public:
        AXLE_TEST_API Entities();

        /**
         * Registers a component for later use in entities
         */
        template <typename T>
        void RegisterComponent();

        /**
         * Creates a new entity and returns a reference to the entities
         * class so that you can add components to it via the WithComponent method.
         *
         * @returns A reference to the entities for immediate use
         */
        AXLE_TEST_API Entities& CreateEntity();

        /**
         * Adds a component to the entity that is currently being created.
         *
         * @param component Component to be added.
         *
         * @returns A reference to the entities class so that you can chain calls to this method.
         */
        template <typename T>
        Entities& WithComponent(T component);

        /**
         * Adds a component to the entity with the given ID.
         *
         * @param id The ID of the entity to add the component to.
         * @param component Component to be added.
         */
        template <typename T>
        void Add(EntityID id, T component);

        /**
         * Deletes a component of type T from the entity with the given ID.
         *
         * @param id The ID of the entity to remove the component from.
         */
        template <typename T>
        void Remove(EntityID id);

        /**
         * Deletes an entity and all its components.
         *
         * @param id The ID of the entity to be deleted.
         */
        AXLE_TEST_API void DeleteEntity(EntityID id);

        /**
         * Gets a reference to the component of type T from the entity with the given ID.
         * Not efficient when using frequently, use with caution.
         *
         * @param id The ID of the entity to get the component from.
         *
         * @returns A reference to the component of type T.
         */
        template <typename T>
        T& Get(EntityID id);

        /**
         * Gets the id of the last created entity.
         *
         * @returns The ID of the last created entity.
         */
        inline EntityID GetLastCreatedEntity() const noexcept {
            return m_InsertingIntoIndex;
        }

        /**
         * Checks if the entity with the given ID has a component of type T.
         *
         * @param id The ID of the entity to check.
         *
         * @returns True if the entity has the component, false otherwise.
         */
        template <typename T>
        inline bool Has(EntityID id) {
            return GetComponentBit<T>(GetMask(id));
        }

        /**
         * Checks if the entity with the given ID has all the components of the given types.
         *
         * @param id The ID of the entity to check.
         *
         * @returns True if the entity has all the components, false otherwise.
         */
        template <typename... Ts>
        inline bool HasAll(EntityID id) {
            return (Has<Ts>(id) && ...);
        }

        /**
         * Checks if the entity with the given ID has at least one component of the given types.
         *
         * @param id The ID of the entity to check.
         *
         * @returns True if the entity has at least one of the components, false otherwise.
         */
        template <typename... Ts>
        inline bool HasAny(EntityID id) {
            return (Has<Ts>(id) || ...);
        }

#ifdef AXLE_TESTING
        inline std::unordered_map<ComponentType, std::unique_ptr<ISparseSet>>& GetComponentArraysTEST() {
            return m_ComponentArrays;
        }

        inline std::array<ComponentMask, MAX_ENTITIES>& GetEntityMasksTEST() {
            return m_EntityMasks;
        }

        std::priority_queue<EntityID, std::vector<EntityID>, std::greater<EntityID>>& GetAvailableEntitiesTEST() {
            return m_AvailableEntities;
        }
#endif // AXLE_TESTING

    private:
        template <typename...>
        friend class View;

        /**
         * Returns the type of a component that has been registered.
         *
         * @returns The type of the component
         */
        template <typename T>
        inline ComponentType GetComponentType() {
            static ComponentType typeID = s_NextComponentType++;
            AX_ASSERT(typeID < MAX_COMPONENTS, "Too many components registered.");
            return typeID;
        }

        /**
         * Set the bit of a component in the component mask of an entity.
         *
         * @param mask The component mask of the entity.
         * @param val The value to set the bit to.
         */
        template <typename T>
        void SetComponentBit(ComponentMask& mask, bool val = true) {
            size_t bitPos = GetComponentType<T>();
            mask.set(bitPos, val);
        }

        /**
         * Gets the bit of a component in the component mask of an entity.
         *
         * @param mask The component mask of the entity.
         *
         * @returns True if the component is enabled, false otherwise.
         */
        template <typename T>
        bool GetComponentBit(const ComponentMask& mask) {
            size_t bitPos = GetComponentType<T>();
            return mask[bitPos];
        }

        /**
         * Returns a generic mask composed of the given component types.
         *
         * @returns A component mask with the bits set for the given component types.
         */
        template <typename... Ts>
        ComponentMask GetComponentMask() {
            ComponentMask mask;
            (SetComponentBit<Ts>(mask), ...);
            return mask;
        }

        /**
         * Gets the component mask of an entity.
         *
         * @param id The ID of the entity.
         *
         * @returns A reference to the component mask of the entity.
         */
        inline ComponentMask& GetMask(EntityID id) {
            AX_ASSERT(m_LivingEntityCount < MAX_ENTITIES,
                      "Entity {0} is out of bounce, the maximum id allowed is: {1}.",
                      id,
                      MAX_ENTITIES - 1);
            return m_EntityMasks.at(id);
        }

        /**
         * Checks if a component of type T has been registered.
         *
         * @returns True if the component has been registered, false otherwise.
         */
        template <typename T>
        inline bool IsComponentRegistered() {
            return m_ComponentArrays.find(GetComponentType<T>()) != m_ComponentArrays.end();
        }

        /**
         * Gets the component array of a given type.
         *
         * @returns A reference to the component array of the given type.
         */
        template <typename T>
        SparseSet<T>& GetComponentArray() {
            ComponentType id = GetComponentType<T>();

            AX_ASSERT(
                IsComponentRegistered<T>(), "Component {0} has not been registered before use.", typeid(T).name());

            return *(static_cast<SparseSet<T>*>(m_ComponentArrays.at(id).get()));
        }

        /**
         * Gets a pointer to the component array of a given type.
         *
         * @returns A pointer to the component array of the given type.
         */
        template <typename T>
        SparseSet<T>* GetComponentArrayPtr() {
            ComponentType id = GetComponentType<T>();

            AX_ASSERT(
                IsComponentRegistered<T>(), "Component {0} has not been registered before use.", typeid(T).name());

            return static_cast<SparseSet<T>*>(m_ComponentArrays.at(id).get());
        }

        /// A hasmap containing the component arrays for every registered component type.
        std::unordered_map<ComponentType, std::unique_ptr<ISparseSet>> m_ComponentArrays;

        /// Stores the next component type to be registered.
        inline static ComponentType s_NextComponentType = 0;

        /// An array of bit masks for every entity.
        ///
        /// The bit mask is used to store which components are enabled for a given
        /// entity.
        ///
        /// For example, if the entity has the first and third registered component, the bit set will be `101`.
        std::array<ComponentMask, MAX_ENTITIES> m_EntityMasks;

        /// The index of the entity that is being inserted into.
        ///
        /// This is used to keep track of which entity is being inserted into when
        /// creating and inserting components in it with the 'WithComponent' method.
        EntityID m_InsertingIntoIndex = 0;

        /// A priority queue of available entity IDs.
        /// When creating an entity the EntityID assigned to it is retrieved from this.
        std::priority_queue<EntityID, std::vector<EntityID>, std::greater<EntityID>> m_AvailableEntities;

        /// The number of living entities in the ECS.
        EntityID m_LivingEntityCount = 0;
    };

    template <typename... Components>
    class View {
    public:
        using FuncView = std::function<void(Components&...)>;
        using FuncViewID = std::function<void(EntityID, Components&...)>;

        AXLE_TEST_API View(Entities* entities)
            : m_Entities(entities),
              m_ComponentArrays(MakeArrayVec<Components...>(entities)) {}

        AXLE_TEST_API void ForEach(const FuncView& func) {
            size_t index = GetSmallestComponentArrayIndex();
            std::vector<EntityID> entities = m_ComponentArrays.at(index)->GetList();

            for (const EntityID& entity : entities) {
                if (m_Entities->HasAll<Components...>(entity)) {
                    func(m_Entities->Get<Components>(entity)...);
                }
            }
        }

        AXLE_TEST_API void ForEach(const FuncViewID& func) {
            size_t index = GetSmallestComponentArrayIndex();
            std::vector<EntityID> entities = m_ComponentArrays.at(index)->GetList();

            for (const EntityID& entity : entities) {
                if (m_Entities->HasAll<Components...>(entity)) {
                    func(entity, m_Entities->Get<Components>(entity)...);
                }
            }
        }

    private:
        size_t GetSmallestComponentArrayIndex() {
            // Find the smallest number of entities that satisfies all the componenents requirements
            size_t smallest_size = std::numeric_limits<size_t>::max();
            size_t index = 0;

            for (size_t i = 0; i < m_ComponentArrays.size(); i++) {
                if (m_ComponentArrays.at(i)->Size() < smallest_size) {
                    smallest_size = m_ComponentArrays.at(i)->Size();
                    index = i;
                }
            }

            AX_ASSERT(smallest_size > 0, "No entities found with the required components.");

            return index;
        }

        template <typename... Cs>
        static std::vector<ISparseSet*> MakeArrayVec(Entities* entities) {
            return {static_cast<ISparseSet*>(entities->GetComponentArrayPtr<Cs>())...};
        }

        std::vector<ISparseSet*> m_ComponentArrays;
        Entities* m_Entities;
    };

#ifdef AXLE_TESTING
    struct Position {
        f32 x, y;
        Position(f32 x = 0.0f, f32 y = 0.0f)
            : x(x),
              y(y) {}
    };

    struct Velocity {
        f32 vx, vy;
        Velocity(f32 vx = 0.0f, f32 vy = 0.0f)
            : vx(vx),
              vy(vy) {}
    };
#endif
} // namespace Axle
