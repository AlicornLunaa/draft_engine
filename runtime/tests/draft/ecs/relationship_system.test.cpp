#include <gtest/gtest.h>
#include "draft/ecs/entity.hpp"
#include "draft/ecs/relationship_components.hpp"
#include "draft/ecs/scene.hpp"

using namespace Draft;

TEST(RelationshipSystem, AddingChildComponentRegistersWithParent)
{
    Scene scene;
    Entity parent = scene.create_entity();
    Entity child = scene.create_entity();

    child.add_component<ChildComponent>(ChildComponent{parent});

    ASSERT_TRUE(parent.has_component<ParentComponent>());
    auto& children = parent.get_component<ParentComponent>().children;
    ASSERT_EQ(children.size(), 1u);
    ASSERT_EQ(children[0], child);
}

TEST(RelationshipSystem, AddingParentComponentBackfillsChildComponents)
{
    Scene scene;
    Entity parent = scene.create_entity();
    Entity child = scene.create_entity();

    ParentComponent pc;
    pc.children.push_back(child);
    parent.add_component<ParentComponent>(pc);

    ASSERT_TRUE(child.has_component<ChildComponent>());
    ASSERT_EQ(child.get_component<ChildComponent>().parent, parent);
}

TEST(RelationshipSystem, RemovingChildComponentUnregistersFromParent)
{
    Scene scene;
    Entity parent = scene.create_entity();
    Entity child = scene.create_entity();
    child.add_component<ChildComponent>(ChildComponent{parent});

    child.remove_component<ChildComponent>();

    // The parent's ParentComponent should be gone too, since it had no other children.
    ASSERT_FALSE(parent.has_component<ParentComponent>());
}

TEST(RelationshipSystem, RemovingOneOfMultipleChildrenKeepsTheOthers)
{
    Scene scene;
    Entity parent = scene.create_entity();
    Entity childA = scene.create_entity();
    Entity childB = scene.create_entity();

    childA.add_component<ChildComponent>(ChildComponent{parent});
    childB.add_component<ChildComponent>(ChildComponent{parent});
    ASSERT_EQ(parent.get_component<ParentComponent>().children.size(), 2u);

    childA.remove_component<ChildComponent>();

    ASSERT_TRUE(parent.has_component<ParentComponent>());
    auto& children = parent.get_component<ParentComponent>().children;
    ASSERT_EQ(children.size(), 1u);
    ASSERT_EQ(children[0], childB);
}

TEST(RelationshipSystem, DestroyingParentDestroysChildren)
{
    Scene scene;
    Entity parent = scene.create_entity();
    Entity child = scene.create_entity();
    child.add_component<ChildComponent>(ChildComponent{parent});

    parent.destroy();

    ASSERT_FALSE(child.is_valid());
}
