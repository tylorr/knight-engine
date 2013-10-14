#include "gtest/gtest.h"
#include "entity_manager.h"

using knight::EntityManager;
using knight::Entity;
using knight::EntityID;

class EntityManagerTest : public ::testing::Test {
 protected:
  EntityManager em_;
};

TEST_F(EntityManagerTest, CreateEntity) {
  EntityID id = em_.CreateEntity();
  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);

  EntityID id2 = em_.CreateEntity();
  EXPECT_EQ(1u, id2.index);
  EXPECT_EQ(0u, id2.version);
}

TEST_F(EntityManagerTest, GetEntity) {
  EntityID id = em_.CreateEntity();
  Entity *entity = em_.GetEntity(id);

  ASSERT_NE(nullptr, entity);
  EXPECT_EQ(id, entity->id());

  EntityID fakeID;
  fakeID.index = 10;
  fakeID.version = 10;

  entity = em_.GetEntity(fakeID);
  EXPECT_EQ(nullptr, entity);

  delete entity;
}

TEST_F(EntityManagerTest, DestroyEntity) {
  EntityID id = em_.CreateEntity();
  em_.DestroyEntity(id);

  Entity *entity = em_.GetEntity(id);
  EXPECT_EQ(nullptr, entity);

  delete entity;
}

TEST_F(EntityManagerTest, UniqueIDs) {
  EntityID id = em_.CreateEntity();
  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);

  em_.DestroyEntity(id);

  EntityID id2 = em_.CreateEntity();
  EXPECT_NE(id, id2);
  EXPECT_EQ(0u, id2.index);
  EXPECT_EQ(1u, id2.version);
}
