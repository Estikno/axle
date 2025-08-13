#include <doctest.h>

#include <typeindex>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <vector>
#include <bitset>

#include "Core/Types.hpp"
#include "ECS/Entities.hpp"
#include "Core/Logger/Log.hpp"
#include "ECS/Systems.hpp"

using namespace Axle;

TEST_CASE("ECS - Systems") {
    Log::Init();
    Entities entities;
    Systems systems;

    entities.RegisterComponent<Position>();
    entities.RegisterComponent<Velocity>();

    entities.CreateEntity().WithComponent<Position>(Position(0.0f, 0.0f)).WithComponent<Velocity>(Velocity(1.0f, 1.0f));
    entities.CreateEntity().WithComponent<Position>(Position(0.0f, 0.0f)).WithComponent<Velocity>(Velocity(2.0f, 0.0f));
    entities.CreateEntity().WithComponent<Position>(Position(5.0f, 5.0f));

    SUBCASE("Add system with single component") {
        systems.Add<Position>([](Position& pos) {
            pos.x += 1.0f;
            pos.y += 1.0f;
        });

        systems.Update(entities);

        View<Position> view(&entities);

        u8 index = 0;
        view.ForEach([&](Position& pos) {
            if (index == 2) {
                CHECK(pos.x == doctest::Approx(6.0f));
                CHECK(pos.y == doctest::Approx(6.0f));
            }
            index++;
        });
    }

    SUBCASE("Simulate loop") {
        systems.Add<Position>([](Position& pos) {
            pos.x += 1.0f;
            pos.y += 1.0f;
        });

        for (int i = 0; i < 10; ++i) {
            systems.Update(entities);
        }

        View<Position> view(&entities);

        u8 index = 0;
        view.ForEach([&](Position& pos) {
            if (index == 2) {
                CHECK(pos.x == doctest::Approx(15.0f));
                CHECK(pos.y == doctest::Approx(15.0f));
            }
            index++;
        });
    }
}
