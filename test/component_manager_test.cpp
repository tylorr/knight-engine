#include "gtest/gtest.h"
#include "component_manager.h"
#include "slot_map.h"
#include "entity.h"
#include "component.h"
#include "common.h"

using namespace knight;

class ComponentManagerTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    Entity::ID id = em_.Create();
    entity_ = em_.Get(id);
  }

  ComponentManager cm_;
  SlotMap em_;
  Entity *entity_;
};

struct TestComponent1 : public Component<TestComponent1> {
 public:
  TestComponent1() { }
};

struct TestComponent2 : public Component<TestComponent2> {
  TestComponent2() { }
};

TEST_F(ComponentManagerTest, AddComponentType) {
  // auto component = cm_.AddComponent<TestComponent1>(entity_);
  // ASSERT_NE(nullptr, component);
  // EXPECT_TRUE(entity_->HasComponent(component));
}

TEST_F(ComponentManagerTest, AddComponentObject) {
  // std::shared_ptr<TestComponent1> component(new TestComponent1());

  // cm_.AddComponent(entity_, component);
  // ASSERT_NE(nullptr, component);
  // EXPECT_TRUE(entity_->HasComponent(component));
}

TEST_F(ComponentManagerTest, GetComponent) {
  // auto c1 = cm_.AddComponent<TestComponent1>(entity_);

  // auto c2 = cm_.GetComponent<TestComponent1>(entity_);
  // EXPECT_EQ(c1, c2);

  // auto c3 = cm_.GetComponent<TestComponent2>(entity_);
  // EXPECT_EQ(nullptr, c3);
}

TEST_F(ComponentManagerTest, RemoveComponentType) {
  // auto c1 = cm_.AddComponent<TestComponent1>(entity_);
  // cm_.RemoveComponent<TestComponent1>(entity_);

  // auto c2 = cm_.GetComponent<TestComponent1>(entity_);
  // EXPECT_EQ(nullptr, c2);
  // EXPECT_FALSE(entity_->HasComponent(c1));
}

TEST_F(ComponentManagerTest, RemoveComponentObject) {
  // auto c1 = cm_.AddComponent<TestComponent1>(entity_);
  // cm_.RemoveComponent(entity_, c1);

  // auto c2 = cm_.GetComponent<TestComponent1>(entity_);
  // EXPECT_EQ(nullptr, c2);
  // EXPECT_FALSE(entity_->HasComponent(c1));
}
