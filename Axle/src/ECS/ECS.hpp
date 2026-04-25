#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

#include "Core/Error/Panic.hpp"
#include "Other/CustomTypes/Expected.hpp"
#include <stdexcept>

namespace Axle {
#ifdef AX_DEBUG
    // This is used for displaying data on the ImGui debug UI
    struct ComponentDescriptorDebug {
        std::string name;
        std::function<void(void*)> drawImGui;
        std::function<void(EntityID)> addToEntity;
        std::function<void(EntityID)> removeFromEntity;
    };

#endif // AX_DEBUG
    class ECS {
    public:
        AXLE_TEST_API ECS();

        /**
         * Initializes the ECS and its singleton
         *
         * Important: This has to be called before using the macros and any other functionality.
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         *
         * It is safe to call multiple times, it simply displays a warning after the first call.
         */
        static void Init();

        /**
         * Shutdowns the manager, important to call when no other component depends on it anymore.
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         */
        static void ShutDown();

        /**
         * Gets the ECS singleton
         *
         * The manager has to have already been initilized before getting the instance.
         *
         * @returns Returns a reference to the Event Handler
         */
        inline static ECS& GetInstance() {
            return *m_ECS;
        }

        /**
         * Registers a component for later use in entities
         */
        template <typename T>
        void RegisterComponent();

#ifdef AX_DEBUG
        /**
         * Registers a component for later use in entities
         *
         * This is the debug method that is used to display the components contents in IMGUI
         */
        template <typename T>
        void RegisterComponent(std::function<void(T&)> editor) {
            // Original version for the normal funtionality
            RegisterComponent<T>();

            ComponentType typeID = GetComponentType<T>();

            std::scoped_lock lock(m_ComponentDescriptorsDebugMutex);
            m_ComponentDescriptorsDebug[typeID] = {
                .name = typeid(T).name(),
                .drawImGui = [editor](void* ptr) { editor(*static_cast<T*>(ptr)); },
                .addToEntity = [this](EntityID id) { Add(id, T{}); },
                .removeFromEntity = [this](EntityID id) { Remove<T>(id); },
            };
        }
#endif // AX_DEBUG

        /**
         * Creates a new entity and returns a reference to the entities
         * class so that you can add components to it via the WithComponent method.
         *
         * @returns A reference to the entities for immediate use
         */
        AXLE_TEST_API ECS& CreateEntity();

        /**
         * Adds a component to the entity that is currently being created.
         *
         * @param component Component to be added.
         *
         * @returns A reference to the entities class so that you can chain calls to this method.
         */
        template <typename T>
        inline ECS& WithComponent(T component) {
            std::scoped_lock lock(m_EntitiesMutex, m_ComponentsMutex);
            AddUnsafe<T>(m_InsertingIntoIndex, component);
            return *this;
        }

        // TODO: Optmize the way components are stored. Because now we double copy them.
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
        Expected<std::reference_wrapper<T>> Get(EntityID id);

        /**
         * Gets the id of the last created entity.
         *
         * @returns The ID of the last created entity.
         */
        inline EntityID GetLastCreatedEntity() const noexcept {
            std::scoped_lock lock(m_EntitiesMutex);
            return m_InsertingIntoIndex;
        }

        /**
         * Checks if the entity with the given ID has a component of type T.
         *
         * Important: Right now if an incorrect entity ID is passed (id out of bounce or not living entity), this will
         * return false. For example, if you pass an entity ID that is out of bounds, this will also return false.
         *
         * @param id The ID of the entity to check.
         *
         * @returns True if the entity has the component, false otherwise.
         */
        template <typename T>
        inline bool Has(EntityID id) {
            std::scoped_lock lock(m_EntitiesMutex, m_ComponentsMutex);
            return HasUnsafe<T>(id);
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
            std::scoped_lock lock(m_EntitiesMutex, m_ComponentsMutex);
            return HasAllUnsafe<Ts...>(id);
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
            std::scoped_lock lock(m_EntitiesMutex, m_ComponentsMutex);
            return HasAnyUnsafe<Ts...>(id);
        }

        /**
         * Checks if a given entity is alive or not (i.e. is initialized)
         *
         * @returns true if the entity is alive and false otherwise
         */
        inline bool IsAlive(EntityID id) const noexcept {
            // If it's not a valid id don't event bother locking the mutex
            if (id >= MAX_ENTITIES)
                return false;

            std::scoped_lock lock(m_EntitiesMutex);
            return m_LivingEntities.at(id);
        }

#ifdef AXLE_TESTING
        inline std::unordered_map<ComponentType, std::unique_ptr<ISparseSet>>& GetComponentArraysTEST() {
            return m_ComponentArrays;
        }

        inline std::array<ComponentMask, MAX_ENTITIES>& GetEntityMasksTEST() {
            return m_EntityMasks;
        }

        inline std::priority_queue<EntityID, std::vector<EntityID>, std::greater<EntityID>>&
        GetAvailableEntitiesTEST() {
            return m_AvailableEntities;
        }

        inline std::unordered_map<ComponentType, ComponentDescriptorDebug>& GetDescriptors() {
            return m_ComponentDescriptorsDebug;
        }
#endif // AXLE_TESTING

    private:
        template <typename...>
        friend class View;

        // TODO: Optmize the way components are stored. Because now we double copy them.
        /**
         * This is the unsafe/unlocked version of the Add method.
         *
         * @param id The ID of the entity to add the component to.
         * @param component Component to be added.
         */
        template <typename T>
        void AddUnsafe(EntityID id, T component);

        /**
         * This is the unsafe/unlocked version of the Remove method.
         *
         * @param id The ID of the entity to remove the component from.
         */
        template <typename T>
        void RemoveUnsafe(EntityID id);

        /**
         * This is the unsafe/unlocked version of the Get method.
         *
         * @param id The ID of the entity to get the component from.
         *
         * @returns A reference to the component of type T.
         */
        template <typename T>
        Expected<std::reference_wrapper<T>> GetUnsafe(EntityID id);

        /**
         * This is the unsafe/unlocked version of the DeleteEntity method.
         *
         * @param id The ID of the entity to be deleted.
         */
        void DeleteEntityUnsafe(EntityID id);

        /**
         * This is the unsafe/unlocked version of the Has method.
         *
         * @param id The ID of the entity to check.
         *
         * @returns True if the entity has the component, false otherwise.
         */
        template <typename T>
        inline bool HasUnsafe(EntityID id) {
            AX_ENSURE(
                IsComponentRegistered<T>(), "Component {0} has not been registered before use.", typeid(T).name());

            Expected<std::reference_wrapper<ComponentMask>> mask = GetMask(id);

            if (!mask.IsValid()) {
                return false;
            }

            return GetComponentBit<T>(mask.Unwrap().get());
        }

        /**
         * This is the unsafe/unlocked version of the HasAll method.
         *
         * @param id The ID of the entity to check.
         *
         * @returns True if the entity has all the components, false otherwise.
         */
        template <typename... Ts>
        inline bool HasAllUnsafe(EntityID id) {
            return (HasUnsafe<Ts>(id) && ...);
        }

        /**
         * This is the unsafe/unlocked version of the HasAny method.
         *
         * @param id The ID of the entity to check.
         *
         * @returns True if the entity has at least one of the components, false otherwise.
         */
        template <typename... Ts>
        inline bool HasAnyUnsafe(EntityID id) {
            return (HasUnsafe<Ts>(id) || ...);
        }

        /**
         * Returns the type of a component that has been registered. This method is thread safe.
         *
         * @returns The type of the component
         */
        template <typename T>
        inline ComponentType GetComponentType() {
            std::scoped_lock lock(s_StaticMutex);

            static ComponentType typeID = s_NextComponentType++;
            AX_ENSURE(typeID < MAX_COMPONENTS, "Too many components registered. The maximum is {0}.", MAX_COMPONENTS);
            return typeID;
        }

        /**
         * Set the bit of a component in the component mask of an entity. This method is not thread safe if used with
         * stored component masks.
         *
         * @param mask The component mask of the entity.
         * @param val The value to set the bit to.
         */
        template <typename T>
        void SetComponentBit(ComponentMask& mask, bool val = true) {
            AX_ENSURE(
                IsComponentRegistered<T>(), "Component {0} has not been registered before use.", typeid(T).name());

            size_t bitPos = GetComponentType<T>();
            mask.set(bitPos, val);
        }

        /**
         * Gets the bit of a component in the component mask of an entity. This method is not thread safe if used with
         * stored component masks.
         *
         * @param mask The component mask of the entity.
         *
         * @returns True if the component is enabled, false otherwise.
         */
        template <typename T>
        bool GetComponentBit(const ComponentMask& mask) {
            AX_ENSURE(
                IsComponentRegistered<T>(), "Component {0} has not been registered before use.", typeid(T).name());

            size_t bitPos = GetComponentType<T>();
            // It will never cause undefined behavior because we ensure that the component is registered,
            // and when a component is registered, its type ID is always less than MAX_COMPONENTS.
            return mask[bitPos];
        }

        /**
         * Returns a generic mask composed of the given component types. This method is thread safe.
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
         * Gets the component mask of an entity. This method is not thread safe.
         *
         * @param id The ID of the entity.
         *
         * @returns A reference to the component mask of the entity.
         */
        inline Expected<std::reference_wrapper<ComponentMask>> GetMask(EntityID id) {
            // AX_ASSERT(id < MAX_ENTITIES,
            //           "Entity {0} is out of bounce, the maximum id allowed is: {1}.",
            //           id,
            //           MAX_ENTITIES - 1);
            if (id >= MAX_ENTITIES) {
                return Expected<std::reference_wrapper<ComponentMask>>::FromException(
                    std::out_of_range("EntityID is out of range"));
            }

            if (!m_LivingEntities.at(id)) {
                return Expected<std::reference_wrapper<ComponentMask>>::FromException(
                    std::invalid_argument("Entity is not alive"));
            }

            return std::ref(m_EntityMasks.at(id));
        }

        /**
         * Checks if a component of type T has been registered. This method is not thread safe.
         *
         * @returns True if the component has been registered, false otherwise.
         */
        template <typename T>
        inline bool IsComponentRegistered() {
            // FIX: If a component has not been registered, its type ID will be registered for later use even if it's
            // not desired
            return m_ComponentArrays.find(GetComponentType<T>()) != m_ComponentArrays.end();
        }

        /**
         * Gets the component array of a given type. This method is not thread safe.
         *
         * @returns A reference to the component array of the given type.
         */
        template <typename T>
        SparseSet<T>& GetComponentArray() {
            return *(GetComponentArrayPtr<T>());
        }

        /**
         * Gets a pointer to the component array of a given type. This method is not thread safe.
         *
         * @returns A pointer to the component array of the given type.
         */
        template <typename T>
        SparseSet<T>* GetComponentArrayPtr() {
            ComponentType id = GetComponentType<T>();

            AX_ENSURE(
                IsComponentRegistered<T>(), "Component {0} has not been registered before use.", typeid(T).name());

            return static_cast<SparseSet<T>*>(m_ComponentArrays.at(id).get());
        }

        /// A hasmap containing the component arrays for every registered component type.
        /// TODO: Because ComponentType is just a number, we could use a vector instead of a hashmap. Or an array given
        /// that we now the maximum number of components at compile time.
        std::unordered_map<ComponentType, std::unique_ptr<ISparseSet>> m_ComponentArrays;
        std::mutex m_ComponentsMutex;

        /// Stores the next component type to be registered.
        inline static ComponentType s_NextComponentType = 0;
        static std::mutex s_StaticMutex;

        /// An array of bit masks for every entity.
        ///
        /// The bit mask is used to store which components are enabled for a given
        /// entity.
        ///
        /// For example, if the entity has the first and third registered component, the bit set will be `101`.
        std::array<ComponentMask, MAX_ENTITIES> m_EntityMasks;

        /// An array that keeps track of which entities are alive and which are not.
        // TODO: Change array to vector, or atleast consider it if there can be a lot of entities
        std::array<bool, MAX_ENTITIES> m_LivingEntities;

        /// The index of the entity that is being inserted into.
        ///
        /// This is used to keep track of which entity is being inserted into when
        /// creating and inserting components in it with the 'WithComponent' method.
        EntityID m_InsertingIntoIndex = 0;

        /// A priority queue of available entity IDs.
        /// When creating an entity the EntityID assigned to it is retrieved from this.
        std::priority_queue<EntityID, std::vector<EntityID>, std::greater<EntityID>> m_AvailableEntities;
        std::mutex m_EntitiesMutex;

        /// The number of living entities in the ECS.
        EntityID m_LivingEntityCount = 0;

        /// The singleton of the event handler class
        static std::unique_ptr<ECS> m_ECS;

#ifdef AX_DEBUG
        // TODO: Because ComponentType is just an integer we can change this unordered_map to a vector for better
        // performance
        std::unordered_map<ComponentType, ComponentDescriptorDebug> m_ComponentDescriptorsDebug;
        std::mutex m_ComponentDescriptorsDebugMutex;
#endif // AX_DEBUG
    };

    // template method definitions
    // ---------------------------
    template <typename T>
    void ECS::RegisterComponent() {
        ComponentType typeID = GetComponentType<T>();
        AX_ENSURE(typeID < MAX_COMPONENTS, "Too many components registered. The maximum is {0}.", MAX_COMPONENTS);

        std::scoped_lock lock(m_ComponentsMutex);

        if (m_ComponentArrays.find(typeID) != m_ComponentArrays.end()) {
            AX_CORE_WARN("Component of type {0} is already registered.", typeid(T).name());
            return;
        }

        m_ComponentArrays.insert({typeID, std::make_unique<SparseSet<T>>()});

        AX_CORE_TRACE("Component {0} has been registered.", typeid(T).name());
    }

    template <typename T>
    void ECS::Add(EntityID id, T component) {
        std::scoped_lock lock(m_EntitiesMutex, m_ComponentsMutex);
        AddUnsafe<T>(id, component);
    }

    template <typename T>
    void ECS::AddUnsafe(EntityID id, T component) {
        AX_ENSURE(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);
        AX_ASSERT(m_LivingEntities.at(id), "Entity ID {0} is not alive.", id);

        if (id >= MAX_ENTITIES) {
            AX_CORE_WARN(
                "Component {0} hasn't been added to entity {1} because the ID is out of bounds.", typeid(T).name(), id);
            return;
        }

        if (!m_LivingEntities.at(id)) {
            AX_CORE_WARN(
                "Component {0} hasn't been added to entity {1} because the entity is not alive.", typeid(T).name(), id);
            return;
        }

        // TODO: Consider making the sparset thread safe to avoid multiple problems here
        SparseSet<T>& array = GetComponentArray<T>();
        array.Add(id, component);

        // We can safely unwrap here because we already checked the bounds of the entity ID
        ComponentMask& entityMask = GetMask(id).Unwrap().get();
        SetComponentBit<T>(entityMask, true);

        AX_CORE_TRACE("Component {0} has been added to entity {1}.", typeid(T).name(), id);
    }

    template <typename T>
    void ECS::Remove(EntityID id) {
        std::scoped_lock lock(m_EntitiesMutex, m_ComponentsMutex);
        RemoveUnsafe<T>(id);
    }

    template <typename T>
    void ECS::RemoveUnsafe(EntityID id) {
        AX_ENSURE(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);
        AX_ASSERT(m_LivingEntities.at(id), "Entity ID {0} is not alive.", id);

        if (id >= MAX_ENTITIES) {
            AX_CORE_WARN("Component {0} hasn't been removed from entity {1} because the ID is out of bounds.",
                         typeid(T).name(),
                         id);
            return;
        }

        if (!m_LivingEntities.at(id)) {
            AX_CORE_WARN("Component {0} hasn't been removed from entity {1} because the entity is not alive.",
                         typeid(T).name(),
                         id);
            return;
        }

        SparseSet<T>& array = GetComponentArray<T>();
        array.Remove(id);

        // We can safely unwrap here because we already checked the bounds of the entity ID
        ComponentMask& entityMask = GetMask(id).Unwrap().get();
        SetComponentBit<T>(entityMask, false);

        AX_CORE_TRACE("Component {0} has been removed from entity {1}.", typeid(T).name(), id);
    }

    template <typename T>
    Expected<std::reference_wrapper<T>> ECS::Get(EntityID id) {
        std::scoped_lock lock(m_ComponentsMutex, m_EntitiesMutex);
        return GetUnsafe<T>(id);
    }

    template <typename T>
    Expected<std::reference_wrapper<T>> ECS::GetUnsafe(EntityID id) {
        AX_ENSURE(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);
        AX_ASSERT(m_LivingEntities.at(id), "Entity ID {0} is not alive.", id);

        if (id >= MAX_ENTITIES) {
            return Expected<std::reference_wrapper<T>>::FromException(std::out_of_range("EntityID is out of range"));
        }

        if (!m_LivingEntities.at(id)) {
            return Expected<std::reference_wrapper<T>>::FromException(std::invalid_argument("Entity is not alive"));
        }

        return GetComponentArray<T>().Get(id);
    }
    // ---------------------------

    template <typename... Components>
    class View {
    public:
        AXLE_TEST_API View(ECS& entities)
            : View(&entities) {} // delegates to pointer version

        AXLE_TEST_API View(ECS* entities)
            : m_Entities(entities),
              m_ComponentArrays(MakeArrayVec<Components...>(entities)) {}

        /**
         * Gets all entities that have all the specified components and their components.
         *
         * @returns A pair containing a vector of EntityIDs and a vector of tuples with the components.
         */
        AXLE_TEST_API std::pair<std::vector<EntityID>, std::vector<std::tuple<Components&...>>> GetAll() {
            std::scoped_lock lock(m_Entities->m_ComponentsMutex, m_Entities->m_EntitiesMutex);

            std::vector<EntityID> entities = GetEntitiesUnsafe();
            std::vector<std::tuple<Components&...>> components;

            for (const EntityID& entity : entities) {
                if (m_Entities->HasAllUnsafe<Components...>(entity)) {
                    // We can safely unwrap here because we already checked that the entity has all the components
                    components.emplace_back((m_Entities->GetUnsafe<Components>(entity)).Unwrap().get()...);
                }
            }

            return {std::move(entities), std::move(components)};
        }

        /**
         * Only gets the components of the entities that have all the specified components.
         *
         * @returns A vector of tuples with the components of the entities that have all the specified components.
         */
        AXLE_TEST_API std::vector<std::tuple<Components&...>> GetComponents() {
            std::scoped_lock lock(m_Entities->m_ComponentsMutex, m_Entities->m_EntitiesMutex);

            std::vector<EntityID> entities = GetEntitiesUnsafe();
            std::vector<std::tuple<Components&...>> components;

            for (const EntityID& entity : entities) {
                if (m_Entities->HasAllUnsafe<Components...>(entity)) {
                    // We can safely unwrap here because we already checked that the entity has all the components
                    components.emplace_back((m_Entities->GetUnsafe<Components>(entity)).Unwrap().get()...);
                }
            }

            return components;
        }

    private:
        /**
         * Returns the maximum number of entities which the given set of components
         *
         * IMPORANT: It's not guaranted for all entities to have all the components, it's
         * just an estimation. If you want correct data call GetAll instead
         *
         * @returns A vector of EntityIDs that are most likely to have all components specified
         */
        AXLE_TEST_API std::vector<EntityID> GetEntitiesUnsafe() {
            size_t index = GetSmallestComponentArrayIndexUnsafe();
            return m_ComponentArrays.at(index)->GetList();
        }

        size_t GetSmallestComponentArrayIndexUnsafe() {
            // Find the smallest number of entities that are most likely to satisfy all the componenents requirements
            size_t smallest_size = std::numeric_limits<size_t>::max();
            size_t index = 0;

            for (size_t i = 0; i < m_ComponentArrays.size(); i++) {
                if (m_ComponentArrays.at(i)->Size() < smallest_size) {
                    smallest_size = m_ComponentArrays.at(i)->Size();
                    index = i;
                }
            }

            AX_ASSERT(
                smallest_size != std::numeric_limits<size_t>::max(),
                "(ONLY IN DEBUG) This always has to return a valid index. There might have been an instantiation of a View with 0 components.");

            return index;
        }

        // NOTE: MakeArrayVec locks m_ComponentsMutex, then View::GetAll locks it again
        // These are separate acquisitions at different points in time, so no deadlock — but it's worth being aware that
        // the pointers captured in the constructor are raw ISparseSet* values. If RegisterComponent is called between
        // construction and GetAll, a rehash of m_ComponentArrays could invalidate those pointers. This is probably fine
        // given typical ECS usage (register everything upfront), but worth a comment documenting that assumption.

        /**
         * Simple function to initialize the vector of component arrays.
         * */
        template <typename... Cs>
        static std::vector<ISparseSet*> MakeArrayVec(ECS* entities) {
            std::scoped_lock lock(entities->m_ComponentsMutex);
            return {static_cast<ISparseSet*>(entities->GetComponentArrayPtr<Cs>())...};
        }

        std::vector<ISparseSet*> m_ComponentArrays;
        ECS* m_Entities;
    };
} // namespace Axle

#ifdef AX_DEBUG
/**
 * Registers a component with an ImGui editor lambda.
 *
 * Usage:
 *   AX_REGISTER_COMPONENT(ecs, Transform, {
 *       ImGui::DragFloat3("Position", &c.position.x);
 *   });
 *
 * 'c' is the component instance, available inside the block.
 */
#    define AX_REGISTER_COMPONENT(type, ...) \
        ::Axle::ECS::GetInstance().RegisterComponent<type>([](type & c) __VA_ARGS__)

/**
 * Begins an ImGui component editor block for use inside AX_REGISTER_COMPONENT.
 * Not strictly needed but makes intent clearer if you want named helpers.
 */
#    define AX_COMPONENT_FIELD_FLOAT(label, field) ::ImGui::DragFloat(label, &c.field)
#    define AX_COMPONENT_FIELD_BOOL(label, field) ::ImGui::Checkbox(label, &c.field)
#    define AX_COMPONENT_FIELD_INT(label, field) ::ImGui::DragInt(label, &c.field)
#else
#    define AX_REGISTER_COMPONENT(type, ...) ::Axle::ECS::GetInstance().RegisterComponent<type>()

// All field helpers expand to nothing in release
#    define AX_COMPONENT_FIELD_FLOAT(label, field)
#    define AX_COMPONENT_FIELD_BOOL(label, field)
#    define AX_COMPONENT_FIELD_INT(label, field)
#endif // AX_DEBUG
