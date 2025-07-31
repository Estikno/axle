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

	SUBCASE("Entity map updates") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Velocity>(new Velocity(1.0f, 2.0f));

		CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()) == 2);

		entities.Add<Position>(0, new Position(3.0f, 4.0f));

		CHECK(entities.GetEntityMasksTEST().at(0) == 3);
	}

	SUBCASE("Delete component") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2 = std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Position>(new Position(1.0f, 2.0f))
			.WithComponent<Velocity>(new Velocity(3.0f, 4.0f));

		CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 3);
		CHECK(entities.GetComponentsTEST().size() == 2);

		entities.Remove<Position>(0);

		CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 2);
		CHECK(entities.GetComponentsTEST().size() == 2);
	}

	SUBCASE("Add component to an already existing entity") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Velocity>(new Velocity(1.0f, 2.0f));

		entities.Add<Position>(0, new Position(3.0f, 4.0f));

		Position* pos = static_cast<Position*>(entities.GetComponentsTEST().at(typeID).at(0).get());

		CHECK(pos->x == doctest::Approx(3.0f));
		CHECK(pos->y == doctest::Approx(4.0f));
	}

	SUBCASE("Delete entity") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Velocity>(new Velocity(1.0f, 2.0f));

		entities.DeleteEntity(0);

		CHECK(entities.GetEntityMasksTEST().size() == 1);
		CHECK(entities.GetEntityMasksTEST().at(0).to_ullong() == 0);
	}

	SUBCASE("Created entities are inserted into delete space") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Position>(new Position(0.0f, 0.0f));
		entities
			.CreateEntity()
			.WithComponent<Position>(new Position(1.0f, 1.0f));

		entities.DeleteEntity(0);

		entities
			.CreateEntity()
			.WithComponent<Position>(new Position(2.0f, 2.0f));

		entities
			.CreateEntity()
			.WithComponent<Velocity>(new Velocity(3.0f, 3.0f));

		CHECK(entities.GetEntityMasksTEST().at(0).to_ullong() == 1);

		Position* pos = static_cast<Position*>(entities.GetComponentsTEST().at(typeID).at(0).get());

		CHECK(pos->x == doctest::Approx(2.0f));
		CHECK(pos->y == doctest::Approx(2.0f));

		CHECK(entities.GetEntityMasksTEST().at(2).to_ullong() == 2);
	}

	SUBCASE("Safe deleting a component twice") {
		std::type_index typeID = std::type_index(typeid(Position));

		entities.RegisterComponent<Position>();

		entities
			.CreateEntity()
			.WithComponent<Position>(new Position(0.0f, 0.0f));

		entities.Remove<Position>(0);
		entities.Remove<Position>(0);

		CHECK(entities.GetEntityMasksTEST().at(0).to_ullong() == 0);
	}
}
