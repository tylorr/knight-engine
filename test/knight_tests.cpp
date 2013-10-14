#include "gtest/gtest.h"
#include "entity_manager.h"

using namespace knight;

TEST(EntityManagerTest, CreateEntity) {
  EntityManager em;

  EntityID id = em.CreateEntity();

  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);
}

TEST(EntityManagerTest, GetEntity) {
  EntityManager em;

  EntityID id = em.CreateEntity();
  Entity *entity = em.GetEntity(id);

  // ASSERT_TRUE(entity != nullptr);
  ASSERT_NE(nullptr, entity);
  EXPECT_EQ(id, entity->id());

  EntityID fakeID;
  fakeID.index = 10;
  fakeID.version = 10;

  entity = em.GetEntity(fakeID);
  EXPECT_EQ(nullptr, entity);

  delete entity;
}

TEST(EntityManagerTest, DestroyEntity) {
  EntityManager em;

  EntityID id = em.CreateEntity();
  em.DestroyEntity(id);

  Entity *entity = em.GetEntity(id);
  EXPECT_EQ(nullptr, entity);
}
