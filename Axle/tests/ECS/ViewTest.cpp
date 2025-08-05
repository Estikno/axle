#include <doctest.h>

#include <typeindex>
#include <typeinfo>
#include <memory>

#include "Core/Types.hpp"
#include "ECS/Entities.hpp"
#include "Math/Mathf.hpp"
#include "Core/Logger/Log.hpp"

using namespace Axle;

TEST_CASE("Ecs View tests") {
    Log::Init();
    Entities entities;

    SUBCASE("Retrieve components") {
        entities.RegisterComponent<Position>();
        entities.RegisterComponent<Velocity>();

        entities.CreateEntity().WithComponent<Position>(Position(0.0f, 1.0f));
        entities.CreateEntity()
            .WithComponent<Position>(Position(2.0f, 3.0f))
            .WithComponent<Velocity>(Velocity(2.0f, 3.0f));
        entities.CreateEntity().WithComponent<Position>(Position(0.0f, 1.0f));
        entities.CreateEntity().WithComponent<Velocity>(Velocity(0.0f, 1.0f));

        View<Position, Velocity> view(&entities);

        view.ForEach([](Position& pos, Velocity& vel) {
            CHECK(pos.x == doctest::Approx(2.0f));
            CHECK(pos.y == doctest::Approx(3.0f));
            CHECK(vel.vx == doctest::Approx(2.0f));
            CHECK(vel.vy == doctest::Approx(3.0f));
        });

        {
            // Retrieve a second time with no problem
            View<Position, Velocity> view(&entities);

            view.ForEach([](Position& pos, Velocity& vel) {
                CHECK(pos.x == doctest::Approx(2.0f));
                CHECK(pos.y == doctest::Approx(3.0f));
                CHECK(vel.vx == doctest::Approx(2.0f));
                CHECK(vel.vy == doctest::Approx(3.0f));
            });

        }
    }
}
