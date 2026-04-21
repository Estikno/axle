#include <doctest.h>

#include <typeindex>
#include <typeinfo>
#include <memory>

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

// TODO: Add more test cases

TEST_CASE("Ecs View tests") {
    Log::Init();
    ECS entities;

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

        for (auto& components : view.GetComponents()) {
            CHECK(std::get<0>(components).x == doctest::Approx(2.0f));
            CHECK(std::get<0>(components).y == doctest::Approx(3.0f));
            CHECK(std::get<1>(components).vx == doctest::Approx(2.0f));
            CHECK(std::get<1>(components).vy == doctest::Approx(3.0f));
        }

        {
            // Retrieve a second time with no problem
            View<Position, Velocity> view(&entities);

            for (auto& components : view.GetComponents()) {
                CHECK(std::get<0>(components).x == doctest::Approx(2.0f));
                CHECK(std::get<0>(components).y == doctest::Approx(3.0f));
                CHECK(std::get<1>(components).vx == doctest::Approx(2.0f));
                CHECK(std::get<1>(components).vy == doctest::Approx(3.0f));
            }
        }
    }
}
