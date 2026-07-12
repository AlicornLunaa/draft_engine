#include <gtest/gtest.h>
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/ecs/system.hpp"

using namespace Draft;

namespace {
    struct TickCounter : AbstractSystem {
        int ticks = 0;
        void update(Time) override { ticks++; }
    };

    struct FrameCounter : AbstractSystem {
        int frames = 0;
        void render(Time) override { frames++; }
    };

    struct LifecycleCounter : AbstractSystem {
        int attachCalls = 0;
        int detachCalls = 0;
        void on_attach() override { attachCalls++; }
        void on_detach() override { detachCalls++; }
    };

    struct EventConsumer : AbstractSystem {
        int eventCalls = 0;
        bool on_event(const Event&) override { eventCalls++; return true; }
    };
}

TEST(Scene, CreateEntityReturnsAValidEntityBoundToThisScene)
{
    Scene scene;
    Entity e = scene.create_entity();

    ASSERT_TRUE(e.is_valid());
    ASSERT_EQ(e.get_scene(), &scene);
}

TEST(Scene, EachCreatedEntityIsDistinct)
{
    Scene scene;
    Entity a = scene.create_entity();
    Entity b = scene.create_entity();

    ASSERT_NE(a, b);
}

TEST(Scene, GetRegistryReflectsCreatedEntities)
{
    Scene scene;
    scene.create_entity();
    scene.create_entity();

    ASSERT_EQ(scene.get_registry().storage<entt::entity>().size(), 2u);
}

TEST(Scene, UpdateDrivesRegisteredSystems)
{
    Scene scene;
    scene.get_systems().add<TickCounter>();

    scene.update(Time::seconds(1.f / 60.f));
    scene.update(Time::seconds(1.f / 60.f));

    ASSERT_EQ(scene.get_systems().get<TickCounter>().ticks, 2);
}

TEST(Scene, RenderDrivesRegisteredSystemsOnceRegardlessOfUpdateCount)
{
    Scene scene;
    scene.get_systems().add<TickCounter>();
    scene.get_systems().add<FrameCounter>();

    // A fixed-step accumulator running three times this frame, but render only once, the same
    // shape a real frame with variable update/render cadences takes.
    scene.update(Time::seconds(1.f / 60.f));
    scene.update(Time::seconds(1.f / 60.f));
    scene.update(Time::seconds(1.f / 60.f));
    scene.render(Time::seconds(1.f / 20.f));

    ASSERT_EQ(scene.get_systems().get<TickCounter>().ticks, 3);
    ASSERT_EQ(scene.get_systems().get<FrameCounter>().frames, 1);
}

TEST(Scene, TwoScenesHaveIndependentRegistriesAndSystems)
{
    Scene a;
    Scene b;

    a.get_systems().add<TickCounter>();
    a.update(Time());

    ASSERT_TRUE(a.get_systems().has<TickCounter>());
    ASSERT_FALSE(b.get_systems().has<TickCounter>());
}

TEST(Scene, AttachAndDetachDriveRegisteredSystemsLifecycleHooks)
{
    Scene scene;
    scene.get_systems().add<LifecycleCounter>();

    scene.attach();
    EXPECT_EQ(scene.get_systems().get<LifecycleCounter>().attachCalls, 1);
    EXPECT_EQ(scene.get_systems().get<LifecycleCounter>().detachCalls, 0);

    scene.detach();
    EXPECT_EQ(scene.get_systems().get<LifecycleCounter>().attachCalls, 1);
    EXPECT_EQ(scene.get_systems().get<LifecycleCounter>().detachCalls, 1);
}

TEST(Scene, DispatchEventReachesRegisteredSystemsAndReturnsWhetherOneConsumedIt)
{
    Scene scene;
    scene.get_systems().add<EventConsumer>();

    Event event;
    event.type = Event::KeyPressed;

    EXPECT_TRUE(scene.dispatch_event(event));
    EXPECT_EQ(scene.get_systems().get<EventConsumer>().eventCalls, 1);
}
