#include <gtest/gtest.h>
#include "draft/components/camera_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/ecs/system.hpp"
#include "draft/rendering/camera.hpp"

#include <memory>

using namespace Draft;

namespace {
    struct TickCounter : AbstractSystem {
        int ticks = 0;
        void update(Time) override { ticks++; }
    };

    struct FrameCounter : AbstractSystem {
        int frames = 0;
        void render(Time, RenderLayer) override { frames++; }
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
    scene.render(Time::seconds(1.f / 20.f), RenderLayer::Default);

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

namespace {
    std::unique_ptr<Camera> make_ortho_camera(float halfExtent = 1.f){
        return std::make_unique<OrthographicCamera>(Vector3f{0, 0, 0}, Vector3f{0, 0, -1}, -halfExtent, halfExtent, -halfExtent, halfExtent);
    }
}

TEST(Scene, NoActiveCameraReturnsNullptr)
{
    Scene scene;
    ASSERT_EQ(scene.get_active_camera(), nullptr);
}

TEST(Scene, InactiveCameraIsIgnored)
{
    Scene scene;
    Entity entity = scene.create_entity();
    entity.add_component<CameraComponent>(CameraComponent{false, 0, make_ortho_camera()});

    ASSERT_EQ(scene.get_active_camera(), nullptr);
}

TEST(Scene, ActiveCameraResolvesToHighestPriority)
{
    Scene scene;

    Entity low = scene.create_entity();
    low.add_component<CameraComponent>(CameraComponent{true, 0, make_ortho_camera(1.f)});

    Entity high = scene.create_entity();
    high.add_component<CameraComponent>(CameraComponent{true, 5, make_ortho_camera(2.f)});

    // The higher-priority (high) camera should be the one returned.
    ASSERT_EQ(scene.get_active_camera(), high.get_component<CameraComponent>().camera.get());
}

TEST(Scene, ActiveCameraSyncsPositionFromTransformComponent)
{
    Scene scene;

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>(TransformComponent{{5.f, 3.f}, 0.f});
    entity.add_component<CameraComponent>(CameraComponent{true, 0, make_ortho_camera()});

    Camera* camera = scene.get_active_camera();
    ASSERT_NE(camera, nullptr);

    ASSERT_FLOAT_EQ(camera->get_position().x, 5.f);
    ASSERT_FLOAT_EQ(camera->get_position().y, 3.f);
}

TEST(Scene, CameraOverrideTakesPrecedenceOverCameraComponent)
{
    Scene scene;

    Entity entity = scene.create_entity();
    entity.add_component<CameraComponent>(CameraComponent{true, 0, make_ortho_camera()});

    std::unique_ptr<Camera> overrideCamera = make_ortho_camera(2.f);
    Camera* overridePtr = overrideCamera.get();
    scene.set_active_camera_override(std::move(overrideCamera));

    ASSERT_EQ(scene.get_active_camera(), overridePtr);
}

TEST(Scene, ClearingCameraOverrideFallsBackToCameraComponent)
{
    Scene scene;

    Entity entity = scene.create_entity();
    entity.add_component<CameraComponent>(CameraComponent{true, 0, make_ortho_camera()});

    scene.set_active_camera_override(make_ortho_camera(2.f));
    scene.set_active_camera_override(nullptr);

    ASSERT_EQ(scene.get_active_camera(), entity.get_component<CameraComponent>().camera.get());
}
