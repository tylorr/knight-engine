#include "gtest/gtest.h"
#include "slot_map.h"
#include "entity.h"
#include "common.h"

using namespace knight;

class SlotMapTest : public ::testing::Test {
 protected:
  SlotMap<Entity> slot_map;
};

TEST_F(SlotMapTest, Create) {
  Entity::ID id = slot_map.Create();
  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);

  Entity::ID id2 = slot_map.Create();
  EXPECT_EQ(1u, id2.index);
  EXPECT_EQ(0u, id2.version);
}

TEST_F(SlotMapTest, Get) {
  Entity::ID id = slot_map.Create();
  Entity *entity = slot_map.Get(id);

  ASSERT_NE(nullptr, entity);
  EXPECT_EQ(id, entity->id());

  Entity::ID fakeID;
  fakeID.index = 10;
  fakeID.version = 10;

  entity = slot_map.Get(fakeID);
  EXPECT_EQ(nullptr, entity);
}

TEST_F(SlotMapTest, Destroy) {
  Entity::ID id = slot_map.Create();
  slot_map.Destroy(id);

  Entity *entity = slot_map.Get(id);
  EXPECT_EQ(nullptr, entity);
}

TEST_F(SlotMapTest, UniqueIDs) {
  Entity::ID id = slot_map.Create();
  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);

  slot_map.Destroy(id);

  Entity::ID id2 = slot_map.Create();
  EXPECT_NE(id, id2);
  EXPECT_EQ(0u, id2.index);
  EXPECT_EQ(1u, id2.version);
}
