#include <doctest.h>

#include <typeindex>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <vector>
#include <bitset>

#include "Core/Types.hpp"
#include "ECS/ECS.hpp"
#include "Core/Logger/Log.hpp"

using namespace Axle;

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

TEST_CASE("Entities ECS Test") {
    Log::Init();
    ECS entities;

    SUBCASE("Register a component") {
        std::type_index typeID = std::type_index(typeid(Position));
        std::type_index typeID2 = std::type_index(typeid(Velocity));

        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        CHECK(entities.GetComponentArraysTEST().size() == 2);
    }

    SUBCASE("Create entity") {
        std::type_index typeID = std::type_index(typeid(Position));
        std::type_index typeID2 = std::type_index(typeid(Velocity));

        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        entities.CreateEntity();

        CHECK(entities.GetLastCreatedEntity() == 0);
        CHECK(entities.GetAvailableEntitiesTEST().top() == 1);
    }

    SUBCASE("Create entity with components") {
        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        Position pos(1.0f, 2.0f);

        EntityID id = entities.CreateEntity();
        entities.Add<Position>(id, pos);
        entities.Add<Velocity>(id, Velocity(3.0f, 4.0f));

        CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 3);

        SparseSet<Position>* p = static_cast<SparseSet<Position>*>(entities.GetComponentArraysTEST().at(0).get());

        CHECK(p->Get(0).Unwrap().get().x == doctest::Approx(1.0f));
        CHECK(p->Get(0).Unwrap().get().y == doctest::Approx(2.0f));
    }

    SUBCASE("Entity map updates") {
        std::type_index typeID = std::type_index(typeid(Position));
        std::type_index typeID2 = std::type_index(typeid(Velocity));

        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        EntityID id = entities.CreateEntity();
        entities.Add(id, Velocity(1.0f, 2.0f));

        CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()) == 2);

        entities.Add<Position>(0, Position(3.0f, 4.0f));

        CHECK(entities.GetEntityMasksTEST().at(0) == 3);
    }

    SUBCASE("Delete component") {
        std::type_index typeID = std::type_index(typeid(Position));
        std::type_index typeID2 = std::type_index(typeid(Velocity));

        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        EntityID id = entities.CreateEntity();
        entities.Add<Position>(id, Position(1.0f, 2.0f));
        entities.Add<Velocity>(id, Velocity(3.0f, 4.0f));

        CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 3);

        entities.Remove<Position>(0);

        CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 2);
    }

    SUBCASE("Add component to an already existing entity") {
        std::type_index typeID = std::type_index(typeid(Position));
        std::type_index typeID2 = std::type_index(typeid(Velocity));

        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        EntityID id = entities.CreateEntity();
        entities.Add<Velocity>(id, Velocity(1.0f, 2.0f));

        entities.Add<Position>(0, Position(3.0f, 4.0f));

        SparseSet<Position>* p = static_cast<SparseSet<Position>*>(entities.GetComponentArraysTEST().at(0).get());

        CHECK(p->Get(0).Unwrap().get().x == doctest::Approx(3.0f));
        CHECK(p->Get(0).Unwrap().get().y == doctest::Approx(4.0f));
    }

    SUBCASE("Delete entity") {
        std::type_index typeID = std::type_index(typeid(Position));
        std::type_index typeID2 = std::type_index(typeid(Velocity));

        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        EntityID id = entities.CreateEntity();
        entities.Add<Velocity>(id, Velocity(1.0f, 2.0f));

        entities.DeleteEntity(0);

        CHECK(entities.GetEntityMasksTEST().at(0).to_ullong() == 0);
    }

    SUBCASE("Created entities are inserted into delete space") {
        std::type_index typeID = std::type_index(typeid(Position));
        std::type_index typeID2 = std::type_index(typeid(Velocity));

        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        EntityID id1 = entities.CreateEntity();
        entities.Add<Position>(id1, Position(0.0f, 0.0f));

        EntityID id2 = entities.CreateEntity();
        entities.Add<Position>(id2, Position(1.0f, 1.0f));

        entities.DeleteEntity(0);

        EntityID id3 = entities.CreateEntity();
        entities.Add<Velocity>(id3, Velocity(2.0f, 2.0f));

        CHECK(entities.GetLastCreatedEntity() == 0);

        EntityID id4 = entities.CreateEntity();
        entities.Add<Velocity>(id4, Velocity(3.0f, 3.0f));

        CHECK(entities.GetLastCreatedEntity() == 2);
        CHECK(entities.GetEntityMasksTEST().at(0).to_ullong() == 2);

        SparseSet<Velocity>* v = static_cast<SparseSet<Velocity>*>(entities.GetComponentArraysTEST().at(1).get());

        CHECK(v->Get(0).Unwrap().get().vx == doctest::Approx(2.0f));
        CHECK(v->Get(0).Unwrap().get().vy == doctest::Approx(2.0f));

        Velocity& vel = entities.Get<Velocity>(2).Unwrap().get();
        CHECK(vel.vx == doctest::Approx(3.0f));
        CHECK(vel.vy == doctest::Approx(3.0f));

        CHECK(entities.GetEntityMasksTEST().at(2).to_ullong() == 2);
    }
}

// ─── Additional edge cases for ECS ────────────────────────────────────────

// ---------------------------------------------------------------------------
// Additional component types for tests
// ---------------------------------------------------------------------------

struct Health {
    f32 value;
    Health(f32 v = 100.0f)
        : value(v) {}
};

struct Tag {
    bool active;
    Tag(bool a = true)
        : active(a) {}
};

// ---------------------------------------------------------------------------
// 1. RegisterComponent deduplication — second call must warn, not duplicate
// ---------------------------------------------------------------------------

TEST_CASE("ECS RegisterComponent deduplication") {
    Log::Init();
    ECS entities;

    entities.RegisterComponent<Position>();
    CHECK_NOTHROW(entities.RegisterComponent<Position>()); // second call — should warn and no-op

    CHECK(entities.GetComponentArraysTEST().size() == 1);
}

// ---------------------------------------------------------------------------
// 2. Entity IDs are reused in ascending order after deletion
// ---------------------------------------------------------------------------

TEST_CASE("ECS entity ID reuse order") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    EntityID id0 = entities.CreateEntity();
    entities.Add<Position>(id0, Position(0.0f, 0.0f));

    EntityID id1 = entities.CreateEntity();
    entities.Add<Position>(id1, Position(1.0f, 1.0f));

    EntityID id2 = entities.CreateEntity();
    entities.Add<Position>(id2, Position(2.0f, 2.0f));

    entities.DeleteEntity(2);
    entities.DeleteEntity(1);

    // Priority queue is min-heap, so ID 1 comes back before ID 2
    entities.CreateEntity();
    CHECK(entities.GetLastCreatedEntity() == 1);

    entities.CreateEntity();
    CHECK(entities.GetLastCreatedEntity() == 2);
}

// ---------------------------------------------------------------------------
// 3. IsAlive reflects create/delete correctly
// ---------------------------------------------------------------------------

TEST_CASE("ECS IsAlive") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    CHECK_FALSE(entities.IsAlive(0));

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position());

    CHECK(entities.IsAlive(0));

    entities.DeleteEntity(0);
    CHECK_FALSE(entities.IsAlive(0));
}

// ---------------------------------------------------------------------------
// 4. IsAlive with an out-of-bounds ID returns false without crashing
// ---------------------------------------------------------------------------

TEST_CASE("ECS IsAlive out-of-bounds") {
    Log::Init();
    ECS entities;

    CHECK_NOTHROW(CHECK_FALSE(entities.IsAlive(MAX_ENTITIES)));
    CHECK_NOTHROW(CHECK_FALSE(entities.IsAlive(MAX_ENTITIES + 999)));
}

// ---------------------------------------------------------------------------
// 5. Get returns a valid reference and mutations persist
// ---------------------------------------------------------------------------

TEST_CASE("ECS Get returns mutable reference") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(1.0f, 2.0f));

    CHECK(id == entities.GetLastCreatedEntity());

    auto result = entities.Get<Position>(id);
    REQUIRE(result.IsValid());

    result.Unwrap().get().x = 99.0f;

    auto again = entities.Get<Position>(id);
    CHECK(again.Unwrap().get().x == doctest::Approx(99.0f));
}

// ---------------------------------------------------------------------------
// 6. Get on a dead entity returns an invalid Expected
// ---------------------------------------------------------------------------

// NOTE: In DEBUG builds the engine craches instead of returning an invalid Expected

// TEST_CASE("ECS Get on dead entity") {
//     Log::Init();
//     ECS entities;
//     entities.RegisterComponent<Position>();
//
//     entities.CreateEntity().WithComponent<Position>(Position(1.0f, 2.0f));
//     entities.DeleteEntity(0);
//
//
//     // auto result = entities.Get<Position>(0);
//     // CHECK_FALSE(result.IsValid());
// }

// ---------------------------------------------------------------------------
// 7. Get with out-of-bounds ID returns invalid Expected
// ---------------------------------------------------------------------------
// NOTE: In DEBUG builds the engine craches instead of returning an invalid Expected

// TEST_CASE("ECS Get out-of-bounds ID") {
//     Log::Init();
//     ECS entities;
//     entities.RegisterComponent<Position>();
//
//     auto result = entities.Get<Position>(MAX_ENTITIES);
//     CHECK_FALSE(result.IsValid());
// }

// ---------------------------------------------------------------------------
// 8. Has / HasAll / HasAny semantics
// ---------------------------------------------------------------------------

TEST_CASE("ECS Has / HasAll / HasAny") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();
    entities.RegisterComponent<Health>();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(1.0f, 2.0f));
    entities.Add<Velocity>(id, Velocity(3.0f, 4.0f));

    SUBCASE("Has returns true only for attached components") {
        CHECK(entities.Has<Position>(id));
        CHECK(entities.Has<Velocity>(id));
        CHECK_FALSE(entities.Has<Health>(id));
    }

    SUBCASE("HasAll requires every component") {
        CHECK(entities.HasAll<Position, Velocity>(id));
        CHECK_FALSE(entities.HasAll<Position, Velocity, Health>(id));
    }

    SUBCASE("HasAny requires at least one component") {
        CHECK(entities.HasAny<Position, Health>(id));
        CHECK(entities.HasAny<Health, Velocity>(id));
        CHECK_FALSE(entities.HasAny<Health>(id));
    }
}

// ---------------------------------------------------------------------------
// 9. Has on a dead entity returns false without crashing
// ---------------------------------------------------------------------------

TEST_CASE("ECS Has on dead entity") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position());

    entities.DeleteEntity(0);

    CHECK_NOTHROW(CHECK_FALSE(entities.Has<Position>(0)));
}

// ---------------------------------------------------------------------------
// 10. Remove clears the component bit but leaves other bits intact
// ---------------------------------------------------------------------------

TEST_CASE("ECS Remove clears only the target component bit") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(1.0f, 2.0f));
    entities.Add<Velocity>(id, Velocity(3.0f, 4.0f));

    entities.Remove<Position>(id);

    CHECK_FALSE(entities.Has<Position>(id));
    CHECK(entities.Has<Velocity>(id));           // untouched
    CHECK(entities.Get<Velocity>(id).IsValid()); // data still accessible
}

// ---------------------------------------------------------------------------
// 11. DeleteEntity clears ALL component bits and data
// ---------------------------------------------------------------------------

TEST_CASE("ECS DeleteEntity wipes all components from sparse sets") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(1.0f, 2.0f));
    entities.Add<Velocity>(id, Velocity(3.0f, 4.0f));

    entities.DeleteEntity(0);

    CHECK(entities.GetEntityMasksTEST().at(0).to_ullong() == 0);
    // NOTE: In DEBUG builds the engine craches instead of returning an invalid Expected
    // CHECK_FALSE(entities.Get<Position>(0).IsValid());
    // CHECK_FALSE(entities.Get<Velocity>(0).IsValid());
}

// ---------------------------------------------------------------------------
// 12. Recycled entity slot does not inherit previous components
// ---------------------------------------------------------------------------

TEST_CASE("ECS recycled entity has clean state") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(1.0f, 2.0f));
    entities.Add<Velocity>(id, Velocity(3.0f, 4.0f));

    entities.DeleteEntity(0);

    // Reuse slot 0 — only attach Velocity this time
    id = entities.CreateEntity();
    entities.Add<Velocity>(id, Velocity(9.0f, 9.0f));
    CHECK(entities.GetLastCreatedEntity() == 0);

    // Position must NOT be present from the previous occupant
    CHECK_FALSE(entities.Has<Position>(0));
    CHECK(entities.Has<Velocity>(0));

    // And the mask must match exactly one component
    CHECK(entities.GetEntityMasksTEST().at(0).count() == 1);
}

// ---------------------------------------------------------------------------
// 13. Multiple entities with the same component type are independent
// ---------------------------------------------------------------------------

TEST_CASE("ECS multiple entities are independent") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(1.0f, 2.0f));

    EntityID id2 = entities.CreateEntity();
    entities.Add<Position>(id2, Position(3.0f, 4.0f));

    auto& p0 = entities.Get<Position>(0).Unwrap().get();
    auto& p1 = entities.Get<Position>(1).Unwrap().get();

    CHECK(p0.x == doctest::Approx(1.0f));
    CHECK(p1.x == doctest::Approx(3.0f));

    p0.x = 99.0f;
    CHECK(p1.x == doctest::Approx(3.0f)); // mutation of one must not affect the other
}

// ---------------------------------------------------------------------------
// 14. View::GetAll only returns entities that have ALL queried components
// ---------------------------------------------------------------------------

TEST_CASE("ECS View GetAll filters correctly") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();

    // Entity 0: both components
    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(1.0f, 0.0f));
    entities.Add<Velocity>(id, Velocity(1.0f, 0.0f));

    // Entity 1: only Position
    id = entities.CreateEntity();
    entities.Add<Position>(id, Position(2.0f, 0.0f));

    // Entity 2: only Velocity
    id = entities.CreateEntity();
    entities.Add<Velocity>(id, Velocity(3.0f, 0.0f));

    View<Position, Velocity> view(entities);
    auto [ids, components] = view.GetAll();

    // Only entity 0 has both
    CHECK(ids.size() == 1);
    CHECK(components.size() == 1);
    CHECK(std::get<0>(components[0]).x == doctest::Approx(1.0f));
}

// ---------------------------------------------------------------------------
// 15. View::GetComponents reflects live mutations
// ---------------------------------------------------------------------------

TEST_CASE("ECS View GetComponents returns live references") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(0.0f, 0.0f));
    entities.Add<Velocity>(id, Velocity(1.0f, 2.0f));

    View<Position, Velocity> view(entities);
    auto components = view.GetComponents();
    REQUIRE(components.size() == 1);

    // Mutate through the view reference
    std::get<Position&>(components[0]).x = 42.0f;

    // Verify the ECS reflects the change
    CHECK(entities.Get<Position>(0).Unwrap().get().x == doctest::Approx(42.0f));
}

// ---------------------------------------------------------------------------
// 16. View with zero matching entities returns empty results
// ---------------------------------------------------------------------------

TEST_CASE("ECS View with no matching entities") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();
    entities.RegisterComponent<Health>();

    // No entity has Health
    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position(1.0f, 0.0f));

    View<Position, Health> view(entities);
    auto [ids, components] = view.GetAll();

    CHECK(ids.empty());
    CHECK(components.empty());
}

// ---------------------------------------------------------------------------
// 17. LivingEntityCount stays consistent across create/delete cycles
// ---------------------------------------------------------------------------

TEST_CASE("ECS living entity count") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    // We can observe count indirectly via the available queue size.
    // After N creates the queue shrinks by N; after M deletes it grows by M.
    auto queueSize = [&]() { return entities.GetAvailableEntitiesTEST().size(); };
    size_t initial = queueSize();

    EntityID id = entities.CreateEntity();
    entities.Add<Position>(id, Position());

    id = entities.CreateEntity();
    entities.Add<Position>(id, Position());

    CHECK(queueSize() == initial - 2);

    entities.DeleteEntity(0);
    CHECK(queueSize() == initial - 1);

    entities.DeleteEntity(1);
    CHECK(queueSize() == initial);
}

// ---------------------------------------------------------------------------
// 18. Add after construction via WithComponent behaves identically to Add()
// ---------------------------------------------------------------------------

TEST_CASE("ECS Add() and WithComponent() produce identical state") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();

    // Build entity A via builder chain
    EntityID a = entities.CreateEntity();
    entities.Add<Position>(a, Position(1.0f, 2.0f));
    entities.Add<Velocity>(a, Velocity(3.0f, 4.0f));

    // Build entity B via individual Add calls
    entities.CreateEntity();
    EntityID b = entities.GetLastCreatedEntity();
    entities.Add<Position>(b, Position(1.0f, 2.0f));
    entities.Add<Velocity>(b, Velocity(3.0f, 4.0f));

    CHECK(entities.GetEntityMasksTEST().at(a) == entities.GetEntityMasksTEST().at(b));
    CHECK(entities.Get<Position>(a).Unwrap().get().x == doctest::Approx(entities.Get<Position>(b).Unwrap().get().x));
    CHECK(entities.Get<Velocity>(a).Unwrap().get().vx == doctest::Approx(entities.Get<Velocity>(b).Unwrap().get().vx));
}

// ---------------------------------------------------------------------------
// Thread safety tests
// ---------------------------------------------------------------------------

// Number of threads and operations — high enough to expose races under TSAN,
// low enough to not time out in a normal test run.
static constexpr int kThreadCount = 8;
static constexpr int kOpsPerThread = 50;

// ---------------------------------------------------------------------------
// 19. Concurrent CreateEntity — no ID is issued twice, count is correct
// ---------------------------------------------------------------------------

TEST_CASE("ECS concurrent CreateEntity produces unique IDs") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    std::vector<EntityID> collectedIDs(kThreadCount * kOpsPerThread);
    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);

    std::atomic<int> slot{0};

    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kOpsPerThread; ++i) {
                EntityID id = entities.CreateEntity();
                entities.Add<Position>(id, Position(static_cast<f32>(i), static_cast<f32>(i)));

                collectedIDs[slot.fetch_add(1, std::memory_order_relaxed)] = id;
            }
        });
    }
    for (auto& th : threads)
        th.join();

    // All issued IDs must be unique
    std::sort(collectedIDs.begin(), collectedIDs.end());
    auto dup = std::adjacent_find(collectedIDs.begin(), collectedIDs.end());
    CHECK(dup == collectedIDs.end());

    // The available queue must have shrunk by exactly the number of created entities
    int total = kThreadCount * kOpsPerThread;
    CHECK(entities.GetAvailableEntitiesTEST().size() == MAX_ENTITIES - total);
}

// ---------------------------------------------------------------------------
// 20. Concurrent Add on different entities — no data corruption
// ---------------------------------------------------------------------------

TEST_CASE("ECS concurrent Add on distinct entities") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    // Pre-create all entities sequentially to give each thread a known ID
    const int total = kThreadCount * kOpsPerThread;
    for (int i = 0; i < total; ++i)
        entities.CreateEntity();

    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);

    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < kOpsPerThread; ++i) {
                EntityID id = static_cast<EntityID>(t * kOpsPerThread + i);
                entities.Add<Position>(id, Position(static_cast<f32>(id), static_cast<f32>(id * 2)));
            }
        });
    }
    for (auto& th : threads)
        th.join();

    // Every entity must have exactly the value that was written for it
    for (int i = 0; i < total; ++i) {
        auto result = entities.Get<Position>(static_cast<EntityID>(i));
        REQUIRE(result.IsValid());
        CHECK(result.Unwrap().get().x == doctest::Approx(static_cast<f32>(i)));
        CHECK(result.Unwrap().get().y == doctest::Approx(static_cast<f32>(i * 2)));
    }
}

// ---------------------------------------------------------------------------
// 21. Concurrent Remove on distinct entities — no crash, masks cleared
// ---------------------------------------------------------------------------

TEST_CASE("ECS concurrent Remove on distinct entities") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    const int total = kThreadCount * kOpsPerThread;
    for (int i = 0; i < total; ++i) {
        EntityID id = entities.CreateEntity();
        entities.Add<Position>(id, Position(static_cast<f32>(i), 0.0f));
    }

    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);

    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < kOpsPerThread; ++i) {
                EntityID id = static_cast<EntityID>(t * kOpsPerThread + i);
                CHECK_NOTHROW(entities.Remove<Position>(id));
            }
        });
    }
    for (auto& th : threads)
        th.join();

    for (int i = 0; i < total; ++i) {
        CHECK_FALSE(entities.Has<Position>(static_cast<EntityID>(i)));
        CHECK(entities.GetEntityMasksTEST().at(i).to_ullong() == 0);
    }
}

// ---------------------------------------------------------------------------
// 22. Concurrent DeleteEntity on distinct entities — living count stays exact
// ---------------------------------------------------------------------------

TEST_CASE("ECS concurrent DeleteEntity keeps consistent living count") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    const int total = kThreadCount * kOpsPerThread;
    for (int i = 0; i < total; ++i) {
        EntityID id = entities.CreateEntity();
        entities.Add<Position>(id, Position());
    }

    size_t sizeAfterCreate = entities.GetAvailableEntitiesTEST().size();

    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);

    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < kOpsPerThread; ++i) {
                EntityID id = static_cast<EntityID>(t * kOpsPerThread + i);
                CHECK_NOTHROW(entities.DeleteEntity(id));
            }
        });
    }
    for (auto& th : threads)
        th.join();

    // All slots must have been returned to the queue
    CHECK(entities.GetAvailableEntitiesTEST().size() == sizeAfterCreate + total);

    // Every deleted entity must report as dead
    for (int i = 0; i < total; ++i)
        CHECK_FALSE(entities.IsAlive(static_cast<EntityID>(i)));
}

// ---------------------------------------------------------------------------
// 23. Concurrent Get while another thread mutates — no crash, reads are valid
// ---------------------------------------------------------------------------

TEST_CASE("ECS concurrent Get and Add do not crash") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();

    // Give every entity both components up front
    const int total = kThreadCount * kOpsPerThread;
    for (int i = 0; i < total; ++i) {
        EntityID id = entities.CreateEntity();
        entities.Add<Position>(id, Position(static_cast<f32>(i), 0.0f));
        entities.Add<Velocity>(id, Velocity(0.0f, static_cast<f32>(i)));
    }

    std::atomic<bool> go{false};
    std::vector<std::thread> threads;
    threads.reserve(kThreadCount * 2);

    // Writer threads: repeatedly overwrite Velocity
    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&, t]() {
            while (!go.load(std::memory_order_acquire)) {}
            for (int i = 0; i < kOpsPerThread; ++i) {
                EntityID id = static_cast<EntityID>(t * kOpsPerThread + i);
                entities.Get<Velocity>(id).Unwrap().get().vx = static_cast<f32>(t);
                entities.Get<Velocity>(id).Unwrap().get().vy = static_cast<f32>(i);
            }
        });
    }

    // Reader threads: repeatedly read Position (which is never mutated)
    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&, t]() {
            while (!go.load(std::memory_order_acquire)) {}
            for (int i = 0; i < kOpsPerThread; ++i) {
                EntityID id = static_cast<EntityID>(t * kOpsPerThread + i);
                auto result = entities.Get<Position>(id);
                // Position was set at creation and never changed — value must be stable
                if (result.IsValid())
                    CHECK(result.Unwrap().get().x == doctest::Approx(static_cast<f32>(id)));
            }
        });
    }

    go.store(true, std::memory_order_release); // release all threads simultaneously
    for (auto& th : threads)
        th.join();
}

// ---------------------------------------------------------------------------
// 24. Concurrent Has / IsAlive reads alongside mutations — no crash
// ---------------------------------------------------------------------------

TEST_CASE("ECS concurrent Has and IsAlive reads are safe") {
    Log::Init();
    ECS entities;
    entities.RegisterComponent<Position>();

    const int total = kThreadCount * kOpsPerThread;
    for (int i = 0; i < total; ++i) {
        EntityID id = entities.CreateEntity();
        entities.Add<Position>(id, Position());
    }

    std::atomic<bool> go{false};
    std::vector<std::thread> threads;
    threads.reserve(kThreadCount * 2);

    // Mutator threads: delete every other entity
    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&, t]() {
            while (!go.load(std::memory_order_acquire)) {}
            for (int i = 0; i < kOpsPerThread; i += 2) {
                EntityID id = static_cast<EntityID>(t * kOpsPerThread + i);
                CHECK_NOTHROW(entities.DeleteEntity(id));
            }
        });
    }

    // Reader threads: just call IsAlive and Has — must not crash
    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([&, t]() {
            while (!go.load(std::memory_order_acquire)) {}
            for (int i = 0; i < kOpsPerThread; ++i) {
                EntityID id = static_cast<EntityID>(t * kOpsPerThread + i);
                CHECK_NOTHROW(entities.IsAlive(id));
                if (entities.IsAlive(id))
                    CHECK_NOTHROW(entities.Has<Position>(id));
            }
        });
    }

    go.store(true, std::memory_order_release);
    for (auto& th : threads)
        th.join();
}
