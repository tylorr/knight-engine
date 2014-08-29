#include "gtest/gtest.h"
#include "entity_bundle.h"
#include "entity_manager.h"
#include "component.h"
#include "entity.h"

#include <memory>

using namespace knight;

class EntityBundleTest : public ::testing::Test {
 protected:
  EntityManager manager_;
};

struct Transform : public Component<Transform> { };
struct TestComponent : public Component<TestComponent> { };

TEST_F(EntityBundleTest, Make) {
  auto transform = std::make_shared<Transform>();
  auto test = std::make_shared<TestComponent>();
  auto bundle = EntityBundle::MakeShared(&manager_, transform, test);

  EXPECT_NE(nullptr, bundle);
}

TEST_F(EntityBundleTest, ConstructShared) {
  auto transform = std::make_shared<Transform>();
  auto test = std::make_shared<TestComponent>();
  auto bundle = EntityBundle::MakeShared(&manager_, transform, test);

  Entity::ID id = bundle->Construct();
  Entity *entity = manager_.Get(id);

  EXPECT_NE(nullptr, entity);
  EXPECT_TRUE(manager_.HasComponent<Transform>(id));
  EXPECT_TRUE(manager_.HasComponent<TestComponent>(id));

  Entity::ID id2 = bundle->Construct();
  auto transform2 = manager_.GetComponent<Transform>(id2);

  // unique entities
  EXPECT_NE(id, id2);

  // same components
  EXPECT_EQ(transform, transform2);
}

TEST_F(EntityBundleTest, ConstructCopy) {

  Entity::ID id;
  std::weak_ptr<Transform> weak_trans;
  {
    auto transform = std::make_shared<Transform>();
    weak_trans = transform;
    auto test = std::make_shared<TestComponent>();
    auto bundle = EntityBundle::MakeCopy(&manager_, transform, test);
    id = bundle->Construct();
  }

  EXPECT_FALSE(weak_trans.lock());

  Entity *entity = manager_.Get(id);

  EXPECT_NE(nullptr, entity);
  EXPECT_TRUE(manager_.HasComponent<Transform>(id));
}
