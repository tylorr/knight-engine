#include "gtest/gtest.h"

#include "common.h"
#include "system.h"
#include "entity.h"
#include "component_manager.h"
#include "component.h"
#include "slot_map.h"

using namespace knight;

class SystemTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    Entity::ID id = em_.Create();
    entity1_ = em_.Get(id);

    id = em_.Create();
    entity2_ = em_.Get(id);
  }

  ComponentManager cm_;
  SlotMap em_;
  System sys_;
  Entity *entity1_;
  Entity *entity2_;
};

class TestComponent1 : public Component<TestComponent1> {
};

class TestComponent2 : public Component<TestComponent2> {
};


TEST_F(SystemTest, SetFlag) {
  // ComponentFlag type1(Component::TypeFor<TestComponent1>());
  // ComponentFlag type2(Component::TypeFor<TestComponent2>());

  // sys_.SetFlag<TestComponent1>();
  // EXPECT_EQ(type1, sys_.component_flags());

  // sys_.SetFlag<TestComponent2>();
  // EXPECT_EQ(type1 | type2, sys_.component_flags());

  // sys_.SetFlag<TestComponent1>();
  // EXPECT_EQ(type1 | type2, sys_.component_flags());
}

TEST_F(SystemTest, OnEntityCreated) {
  // sys_.OnEntityCreated(entity1_);
  // EXPECT_EQ(1, sys_.size());
}
