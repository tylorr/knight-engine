#include "gtest/gtest.h"
#include "slot_map.h"
#include "entity.h"
#include "common.h"

using knight::SlotMap;
using knight::Entity;
using knight::ID;

class SlotMapTest : public ::testing::Test {
 protected:
  SlotMap<Entity> em_;
};

TEST_F(SlotMapTest, Create) {
  ID id = em_.Create();
  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);

  ID id2 = em_.Create();
  EXPECT_EQ(1u, id2.index);
  EXPECT_EQ(0u, id2.version);
}

TEST_F(SlotMapTest, Get) {
  ID id = em_.Create();
  Entity *entity = em_.Get(id);

  ASSERT_NE(nullptr, entity);
  EXPECT_EQ(id, entity->id());

  ID fakeID;
  fakeID.index = 10;
  fakeID.version = 10;

  entity = em_.Get(fakeID);
  EXPECT_EQ(nullptr, entity);
}

TEST_F(SlotMapTest, Destroy) {
  ID id = em_.Create();
  em_.Destroy(id);

  Entity *entity = em_.Get(id);
  EXPECT_EQ(nullptr, entity);
}

TEST_F(SlotMapTest, UniqueIDs) {
  ID id = em_.Create();
  EXPECT_EQ(0u, id.index);
  EXPECT_EQ(0u, id.version);

  em_.Destroy(id);

  ID id2 = em_.Create();
  EXPECT_NE(id, id2);
  EXPECT_EQ(0u, id2.index);
  EXPECT_EQ(1u, id2.version);
}
