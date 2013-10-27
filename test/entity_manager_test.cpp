#include "gtest/gtest.h"
#include "entity_manager.h"
#include "slot_map.h"
#include "entity.h"
#include "component.h"
#include "common.h"

using namespace knight;

class EntityManagerTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    id_ = entity_manager_.Create();
  }

  EntityManager entity_manager_;
  Entity::ID id_;
};

struct TestComponent1 : public Component<TestComponent1> {
 public:
  TestComponent1() : x_(0) { }
  TestComponent1(int x) : x_(x) { }

  int x_;
};

struct TestComponent2 : public Component<TestComponent2> {
  TestComponent2() { }
};


TEST_F(EntityManagerTest, AddComponentType) {
  int x = 3;
  auto component = entity_manager_.AddComponent<TestComponent1>(id_, x);
  ASSERT_NE(nullptr, component);

  EXPECT_EQ(x, component->x_);

  EXPECT_TRUE(entity_manager_.HasComponent<TestComponent1>(id_));
  EXPECT_TRUE(entity_manager_.component_mask(id_).test(TestComponent1::family()));
}

TEST_F(EntityManagerTest, AddComponentObject) {
  auto component = std::make_shared<TestComponent1>();
  entity_manager_.AddComponent(id_, component);

  EXPECT_TRUE(entity_manager_.HasComponent<TestComponent1>(id_));
  EXPECT_TRUE(entity_manager_.component_mask(id_).test(TestComponent1::family()));
}

TEST_F(EntityManagerTest, GetComponent) {
  auto c1 = entity_manager_.AddComponent<TestComponent1>(id_);

  auto c2 = entity_manager_.GetComponent<TestComponent1>(id_);
  EXPECT_EQ(c1, c2);

  auto c3 = entity_manager_.GetComponent<TestComponent2>(id_);
  EXPECT_EQ(nullptr, c3);
}

TEST_F(EntityManagerTest, RemoveComponent) {
  auto c1 = entity_manager_.AddComponent<TestComponent1>(id_);
  entity_manager_.RemoveComponent<TestComponent1>(id_);

  auto c2 = entity_manager_.GetComponent<TestComponent1>(id_);
  EXPECT_EQ(nullptr, c2);
  EXPECT_FALSE(entity_manager_.HasComponent<TestComponent1>(id_));
}
