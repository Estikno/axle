#include <doctest.h>

#include <typeindex>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <tuple>

#include "Core/Types.hpp"
#include "ECS/ECS.hpp"
#include "Core/Logger/Log.hpp"
#include "ECS/Systems.hpp"

using namespace Axle;

TEST_CASE("ECS - Systems") {
    Log::Init();
    ECS entities;
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
        std::vector<std::tuple<Position&>> components = view.GetComponents();

        for (size_t i = 0; i < components.size(); ++i) {
            if (i == 2) {
                // The third entity with only Position
                CHECK(std::get<0>(components[i]).x == doctest::Approx(6.0f));
                CHECK(std::get<0>(components[i]).y == doctest::Approx(6.0f));
            }
        }
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
        std::vector<std::tuple<Position&>> components = view.GetComponents();

        for (size_t i = 0; i < components.size(); ++i) {
            if (i == 2) {
                // The third entity with only Position
                CHECK(std::get<0>(components[i]).x == doctest::Approx(15.0f));
                CHECK(std::get<0>(components[i]).y == doctest::Approx(15.0f));
            }
        }
    }

    SUBCASE("More systems") {
        // For this test, it uses Semi-implicit Euler (Not a real physics, just a simple simulation)
        // The time step (dt) is assumed to be 1.0f for simplicity.

        systems.Add<Velocity>([](Velocity& vel) {
            vel.vx += 1.0f;
            vel.vy += 1.0f;
        });

        systems.Add<Position, Velocity>([](EntityID id, Position& pos, Velocity& vel) {
            pos.x += vel.vx;
            pos.y += vel.vy;
        });


        for (int i = 1; i <= 10; ++i) {
            systems.Update(entities);
        }

        View<Position, Velocity> view(&entities);
        std::vector<std::tuple<Position&, Velocity&>> components = view.GetComponents();

        for (size_t i = 0; i < components.size(); ++i) {
            if (i == 0) {
                // Even thought if we calculate the final position with UARM it should be: 60.0f,
                // because we are using Semi-implicit Euler, the final position will have an error and will be 65.0f
                CHECK(std::get<0>(components[i]).x == doctest::Approx(65.0f));
                CHECK(std::get<0>(components[i]).y == doctest::Approx(65.0f));
                CHECK(std::get<1>(components[i]).vx == doctest::Approx(11.0f));
                CHECK(std::get<1>(components[i]).vy == doctest::Approx(11.0f));
            } else if (i == 1) {
                // Here happens the same, the final position will have an error and will be 75.0f instad of 70.0f
                CHECK(std::get<0>(components[i]).x == doctest::Approx(75.0f));
                CHECK(std::get<0>(components[i]).y == doctest::Approx(55.0f));
                CHECK(std::get<1>(components[i]).vx == doctest::Approx(12.0f));
                CHECK(std::get<1>(components[i]).vy == doctest::Approx(10.0f));
            }
        }

        // Check that the third entity with only Position was not modifie
        View<Position> view_2(&entities);
        std::vector<std::tuple<Position&>> components_2 = view_2.GetComponents();

        for (size_t i = 0; i < components.size(); ++i) {
            if (i == 2) {
                CHECK(std::get<0>(components_2[i]).x == doctest::Approx(5.0f));
                CHECK(std::get<0>(components_2[i]).y == doctest::Approx(5.0f));
            }
        }
    }
}
