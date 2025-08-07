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

		entities
			.CreateEntity()
			.WithComponent<Position>(pos)
			.WithComponent<Velocity>(Velocity(3.0f, 4.0f));

		CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 3);

		SparseSet<Position>* p = static_cast<SparseSet<Position>*>(entities.GetComponentArraysTEST().at(0).get());

		CHECK(p->Get(0).x == doctest::Approx(1.0f));
		CHECK(p->Get(0).y == doctest::Approx(2.0f));
	}

	SUBCASE("Entity map updates") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Velocity>(Velocity(1.0f, 2.0f));

		CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()) == 2);

		entities.Add<Position>(0, Position(3.0f, 4.0f));

		CHECK(entities.GetEntityMasksTEST().at(0) == 3);
	}

	SUBCASE("Delete component") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2 = std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Position>(Position(1.0f, 2.0f))
			.WithComponent<Velocity>(Velocity(3.0f, 4.0f));

		CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 3);

		entities.Remove<Position>(0);

		CHECK(entities.GetEntityMasksTEST().at(entities.GetLastCreatedEntity()).to_ullong() == 2);
	}

	SUBCASE("Add component to an already existing entity") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Velocity>(Velocity(1.0f, 2.0f));

		entities.Add<Position>(0, Position(3.0f, 4.0f));

		SparseSet<Position>* p = static_cast<SparseSet<Position>*>(entities.GetComponentArraysTEST().at(0).get());

		CHECK(p->Get(0).x == doctest::Approx(3.0f));
		CHECK(p->Get(0).y == doctest::Approx(4.0f));
	}

	SUBCASE("Delete entity") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Velocity>(Velocity(1.0f, 2.0f));

		entities.DeleteEntity(0);

		CHECK(entities.GetEntityMasksTEST().at(0).to_ullong() == 0);
	}

	SUBCASE("Created entities are inserted into delete space") {
		std::type_index typeID = std::type_index(typeid(Position));
		std::type_index typeID2= std::type_index(typeid(Velocity));

		entities.RegisterComponent<Position>();
		entities.RegisterComponent<Velocity>();

		entities
			.CreateEntity()
			.WithComponent<Position>(Position(0.0f, 0.0f));
		entities
			.CreateEntity()
			.WithComponent<Position>(Position(1.0f, 1.0f));

		entities.DeleteEntity(0);

		entities
			.CreateEntity()
			.WithComponent<Velocity>(Velocity(2.0f, 2.0f));

		CHECK(entities.GetLastCreatedEntity() == 0);

		entities
			.CreateEntity()
			.WithComponent<Velocity>(Velocity(3.0f, 3.0f));

		CHECK(entities.GetLastCreatedEntity() == 2);
		CHECK(entities.GetEntityMasksTEST().at(0).to_ullong() == 2);

		SparseSet<Velocity>* v = static_cast<SparseSet<Velocity>*>(entities.GetComponentArraysTEST().at(1).get());

		CHECK(v->Get(0).vx == doctest::Approx(2.0f));
		CHECK(v->Get(0).vy == doctest::Approx(2.0f));

		Velocity& vel = entities.Get<Velocity>(2);
		CHECK(vel.vx == doctest::Approx(3.0f));
		CHECK(vel.vy == doctest::Approx(3.0f));

		CHECK(entities.GetEntityMasksTEST().at(2).to_ullong() == 2);
	}
}
