#include <doctest.h>

#include <typeindex>
#include <typeinfo>
#include <memory>
#include <unordered_map>
#include <vector>
#include <bitset>

#include "Core/Types.hpp"
#include "ECS/Entities.hpp"
#include "Math/Mathf.hpp"
#include "Core/Logger/Log.hpp"

using namespace Axle;

TEST_CASE("Entities ECS Test") {
	Log::Init();
	Entities entities;

	SUBCASE("Register a component") {
		std::type_index typeID = std::type_index(typeid(Position));

		entities.RegisterComponent<Position>();

		std::vector<std::shared_ptr<void>>& components = entities.GetComponentsTEST().at(typeID);
		ComponentMask& mask = entities.GetComponentMasksTEST().at(typeID);
		ComponentMask howShouldBe = ComponentMask();
		howShouldBe.set(0);

		CHECK(components.size() == 0);
		CHECK(entities.GetComponentsTEST().size() == 1);
		CHECK(mask == howShouldBe);
	}

	SUBCASE("Bitmask update when registering components") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		std::vector<std::shared_ptr<void>>& components = entities.GetComponentsTEST().at(typeID);

		ComponentMask& mask = entities.GetComponentMasksTEST().at(typeID);
		ComponentMask& mask2 = entities.GetComponentMasksTEST().at(typeID2);

		CHECK(components.size() == 0);
		CHECK(entities.GetComponentsTEST().size() == 2);
		CHECK(mask.to_ullong() == 1);
		CHECK(mask2.to_ullong() == 2);
	}

	SUBCASE("Create entity") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities.CreateEntity();

		std::vector<std::shared_ptr<void>>& components = entities.GetComponentsTEST().at(typeID);
		std::vector<std::shared_ptr<void>>& components2 = entities.GetComponentsTEST().at(typeID2);

		CHECK(entities.GetLastCreatedEntity() == 0);
		CHECK(components.size() == 1);
		CHECK(components.at(0) == nullptr);
		CHECK(components2.size() == 1);
		CHECK(components2.at(0) == nullptr);
	}

	SUBCASE("Create entity with components") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		Position* pos = new Position(1.0f, 2.0f);

		entities
			.CreateEntity()
			.WithComponent<Position>(pos)
			.WithComponent<Velocity>(new Velocity(3.0f, 4.0f));

		CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 3);
		CHECK(entities.GetComponentsTEST().size() == 2);
		CHECK(entities.GetComponentsTEST().at(typeID).size() == 1);

		Position* s = static_cast<Position*>(entities.GetComponentsTEST().at(typeID).at(entities.GetLastCreatedEntity()).get());

		CHECK(s->x == doctest::Approx(1.0f));
		CHECK(s->y == doctest::Approx(2.0f));
	}
}
