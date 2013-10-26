#include "gtest/gtest.h"
#include "slot_map.h"
#include "entity.h"
#include "common.h"

using namespace knight;

class SlotMapTest : public ::testing::Test {
 protected:
  SlotMap em_;
};

TEST_F(SlotMapTest, Create) {
  Entity::ID id = em_.Create();
  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);

  Entity::ID id2 = em_.Create();
  EXPECT_EQ(1u, id2.index);
  EXPECT_EQ(0u, id2.version);
}

TEST_F(SlotMapTest, Get) {
  Entity::ID id = em_.Create();
  Entity *entity = em_.Get(id);

  ASSERT_NE(nullptr, entity);
  EXPECT_EQ(id, entity->id());

  Entity::ID fakeID;
  fakeID.index = 10;
  fakeID.version = 10;

  entity = em_.Get(fakeID);
  EXPECT_EQ(nullptr, entity);
}

TEST_F(SlotMapTest, Destroy) {
  Entity::ID id = em_.Create();
  em_.Destroy(id);

  Entity *entity = em_.Get(id);
  EXPECT_EQ(nullptr, entity);
}

TEST_F(SlotMapTest, UniqueIDs) {
  Entity::ID id = em_.Create();
  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);

  em_.Destroy(id);

  Entity::ID id2 = em_.Create();
  EXPECT_NE(id, id2);
  EXPECT_EQ(0u, id2.index);
  EXPECT_EQ(1u, id2.version);
}
