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
  auto bundle = EntityBundle::Make(&manager_, transform, test);

  EXPECT_NE(nullptr, bundle);
}

TEST_F(EntityBundleTest, Construct) {
  auto transform = std::make_shared<Transform>();
  auto test = std::make_shared<TestComponent>();
  auto bundle = EntityBundle::Make(&manager_, transform, test);

  Entity::ID id = bundle->Construct();
  Entity *entity = manager_.Get(id);

  EXPECT_NE(nullptr, entity);
  EXPECT_TRUE(manager_.HasComponent<Transform>(id));
  EXPECT_TRUE(manager_.HasComponent<TestComponent>(id));
}
