#include <gtest/gtest.h>
#include "draft/ecs/system.hpp"

#include <stdexcept>
#include <string>
#include <vector>

using namespace Draft;

namespace {
    struct RecordingSystem : AbstractSystem {
        std::vector<std::string>* log;
        std::string name;
        Time lastDt;

        RecordingSystem(std::vector<std::string>& log, std::string name) : log(&log), name(std::move(name)) {}

        void update(Time dt) override {
            log->push_back(name);
            lastDt = dt;
        }
    };

    struct CountingSystem : AbstractSystem {
        int calls = 0;
        void update(Time) override { calls++; }
    };
}

TEST(SystemRegistry, AddReturnsAReferenceToTheConstructedSystem)
{
    SystemRegistry systems;
    CountingSystem& s = systems.add<CountingSystem>();
    ASSERT_EQ(s.calls, 0);
}

TEST(SystemRegistry, GetThrowsWhenNotRegistered)
{
    SystemRegistry systems;
    ASSERT_THROW(systems.get<CountingSystem>(), std::logic_error);
}

TEST(SystemRegistry, GetReturnsTheRegisteredSystem)
{
    SystemRegistry systems;
    systems.add<CountingSystem>();

    CountingSystem& s = systems.get<CountingSystem>();
    s.update(Time());
    ASSERT_EQ(systems.get<CountingSystem>().calls, 1);
}

TEST(SystemRegistry, TryGetReturnsNullptrWhenNotRegistered)
{
    SystemRegistry systems;
    ASSERT_EQ(systems.try_get<CountingSystem>(), nullptr);

    systems.add<CountingSystem>();
    ASSERT_NE(systems.try_get<CountingSystem>(), nullptr);
}

TEST(SystemRegistry, Has)
{
    SystemRegistry systems;
    ASSERT_FALSE(systems.has<CountingSystem>());

    systems.add<CountingSystem>();
    ASSERT_TRUE(systems.has<CountingSystem>());
}

TEST(SystemRegistry, Remove)
{
    SystemRegistry systems;
    systems.add<CountingSystem>();
    ASSERT_TRUE(systems.remove<CountingSystem>());
    ASSERT_FALSE(systems.has<CountingSystem>());
}

TEST(SystemRegistry, RemoveOnUnregisteredReturnsFalse)
{
    SystemRegistry systems;
    ASSERT_FALSE(systems.remove<CountingSystem>());
}

TEST(SystemRegistry, UpdateAllRunsEveryRegisteredSystem)
{
    SystemRegistry systems;
    systems.add<CountingSystem>();

    systems.update_all(Time::seconds(0.5f));
    systems.update_all(Time::seconds(0.5f));

    ASSERT_EQ(systems.get<CountingSystem>().calls, 2);
}

TEST(SystemRegistry, UpdateAllPassesDtThrough)
{
    std::vector<std::string> log;
    SystemRegistry systems;
    systems.add<RecordingSystem>(log, "only");

    Time dt = Time::seconds(1.0f / 60.0f);
    systems.update_all(dt);

    ASSERT_EQ(systems.get<RecordingSystem>().lastDt.as_microseconds(), dt.as_microseconds());
}

TEST(SystemRegistry, ReAddingTheSameTypeReplacesItRatherThanDuplicating)
{
    std::vector<std::string> log;
    SystemRegistry systems;

    systems.add<RecordingSystem>(log, "first");
    systems.add<RecordingSystem>(log, "second"); // same type as above so it replaces it

    systems.update_all(Time());
    ASSERT_EQ(log, (std::vector<std::string>{"second"}));
}

namespace {
    struct SystemA : AbstractSystem {
        std::vector<std::string>* log;
        explicit SystemA(std::vector<std::string>& log) : log(&log) {}
        void update(Time) override { log->push_back("A"); }
    };

    struct SystemB : AbstractSystem {
        std::vector<std::string>* log;
        explicit SystemB(std::vector<std::string>& log) : log(&log) {}
        void update(Time) override { log->push_back("B"); }
    };
}

TEST(SystemRegistry, DistinctSystemTypesRunInTheOrderTheyWereFirstAdded)
{
    std::vector<std::string> log;
    SystemRegistry systems;

    systems.add<SystemB>(log);
    systems.add<SystemA>(log);

    systems.update_all(Time());
    ASSERT_EQ(log, (std::vector<std::string>{"B", "A"}));
}

TEST(SystemRegistry, RegisteredTypesListsEveryTypeInRegistrationOrder)
{
    std::vector<std::string> log;
    SystemRegistry systems;
    systems.add<SystemB>(log);
    systems.add<SystemA>(log);

    ASSERT_EQ(systems.registered_types(), (std::vector<std::type_index>{
        std::type_index(typeid(SystemB)),
        std::type_index(typeid(SystemA)),
    }));
}

TEST(SystemRegistry, RegisteredTypesDropsAnEntryOnRemove)
{
    std::vector<std::string> log;
    SystemRegistry systems;
    systems.add<SystemB>(log);
    systems.add<SystemA>(log);
    systems.remove<SystemB>();

    ASSERT_EQ(systems.registered_types(), (std::vector<std::type_index>{
        std::type_index(typeid(SystemA)),
    }));
}

TEST(SystemRegistry, ReplacingASystemKeepsItsOriginalPositionInOrder)
{
    std::vector<std::string> log;
    SystemRegistry systems;

    systems.add<SystemB>(log);
    systems.add<SystemA>(log);
    systems.add<SystemB>(log); // re-added, should not move to the back

    systems.update_all(Time());
    ASSERT_EQ(log, (std::vector<std::string>{"B", "A"}));
}

namespace {
    // Only overrides update(), the fixed-step cadence.
    struct UpdateOnlySystem : AbstractSystem {
        int updateCalls = 0;
        void update(Time) override { updateCalls++; }
    };

    // Only overrides render(), the once-per-frame variable-dt cadence. Never overrides
    // get_render_layers(), so it stays on the default RenderLayer::Default.
    struct RenderOnlySystem : AbstractSystem {
        int renderCalls = 0;
        Time lastDt;
        RenderLayer lastLayer = RenderLayer::None;
        void render(Time dt, RenderLayer layer) override { renderCalls++; lastDt = dt; lastLayer = layer; }
    };
}

TEST(SystemRegistry, RenderAllRunsEveryRegisteredSystemsRenderHook)
{
    SystemRegistry systems;
    systems.add<RenderOnlySystem>();

    systems.render_all(Time::seconds(1.f / 60.f), RenderLayer::Default);
    systems.render_all(Time::seconds(1.f / 60.f), RenderLayer::Default);

    ASSERT_EQ(systems.get<RenderOnlySystem>().renderCalls, 2);
}

TEST(SystemRegistry, RenderAllPassesDtThrough)
{
    SystemRegistry systems;
    systems.add<RenderOnlySystem>();

    Time dt = Time::seconds(1.f / 30.f);
    systems.render_all(dt, RenderLayer::Default);

    ASSERT_EQ(systems.get<RenderOnlySystem>().lastDt.as_microseconds(), dt.as_microseconds());
}

TEST(SystemRegistry, RenderAllOnlyCallsSystemsWhoseLayersIncludeTheRequestedLayer)
{
    SystemRegistry systems;
    systems.add<RenderOnlySystem>(); // RenderLayer::Default only

    systems.render_all(Time::seconds(1.f / 60.f), RenderLayer::Geometry);
    ASSERT_EQ(systems.get<RenderOnlySystem>().renderCalls, 0);

    systems.render_all(Time::seconds(1.f / 60.f), RenderLayer::Default);
    ASSERT_EQ(systems.get<RenderOnlySystem>().renderCalls, 1);
    ASSERT_EQ(systems.get<RenderOnlySystem>().lastLayer, RenderLayer::Default);
}

TEST(SystemRegistry, UpdateAndRenderCadencesAreIndependent)
{
    SystemRegistry systems;
    systems.add<UpdateOnlySystem>();
    systems.add<RenderOnlySystem>();

    // Simulate a frame where the fixed-step accumulator ran three times but render runs once,
    // same as a real frame with a slow physics step and a fast frame rate (or vice versa).
    systems.update_all(Time::seconds(1.f / 60.f));
    systems.update_all(Time::seconds(1.f / 60.f));
    systems.update_all(Time::seconds(1.f / 60.f));
    systems.render_all(Time::seconds(1.f / 30.f), RenderLayer::Default);

    ASSERT_EQ(systems.get<UpdateOnlySystem>().updateCalls, 3);
    ASSERT_EQ(systems.get<RenderOnlySystem>().renderCalls, 1);
}

TEST(SystemRegistry, SystemsWithNoOverrideForACadenceAreHarmlessNoOps)
{
    // UpdateOnlySystem never overrides render(), calling render_all() should just do nothing
    // to it rather than erroring, since AbstractSystem::render() defaults to a no-op.
    SystemRegistry systems;
    systems.add<UpdateOnlySystem>();

    ASSERT_NO_THROW(systems.render_all(Time(), RenderLayer::Default));
    ASSERT_EQ(systems.get<UpdateOnlySystem>().updateCalls, 0);
}

namespace {
    // Overrides on_attach()/on_detach(), the scene-lifecycle cadence (e.g. starting/stopping
    // ambient music when this system's Scene becomes/stops being active).
    struct LifecycleSystem : AbstractSystem {
        int attachCalls = 0;
        int detachCalls = 0;
        void on_attach() override { attachCalls++; }
        void on_detach() override { detachCalls++; }
    };

    // Always consumes whatever event it sees.
    struct ConsumingSystem : AbstractSystem {
        int eventCalls = 0;
        bool on_event(const Event&) override { eventCalls++; return true; }
    };

    // Never consumes - just counts, so a stop-propagation test can confirm it was never reached.
    struct ObservingSystem : AbstractSystem {
        int eventCalls = 0;
        bool on_event(const Event&) override { eventCalls++; return false; }
    };
}

TEST(SystemRegistry, AttachAllRunsEveryRegisteredSystemsOnAttachHookExactlyOnce)
{
    SystemRegistry systems;
    systems.add<LifecycleSystem>();

    systems.attach_all();
    ASSERT_EQ(systems.get<LifecycleSystem>().attachCalls, 1);
    ASSERT_EQ(systems.get<LifecycleSystem>().detachCalls, 0);
}

TEST(SystemRegistry, DetachAllRunsEveryRegisteredSystemsOnDetachHookExactlyOnce)
{
    SystemRegistry systems;
    systems.add<LifecycleSystem>();

    systems.detach_all();
    ASSERT_EQ(systems.get<LifecycleSystem>().detachCalls, 1);
    ASSERT_EQ(systems.get<LifecycleSystem>().attachCalls, 0);
}

TEST(SystemRegistry, DispatchEventReturnsFalseWhenNoSystemConsumesIt)
{
    SystemRegistry systems;
    systems.add<ObservingSystem>();

    Event event;
    event.type = Event::KeyPressed;

    EXPECT_FALSE(systems.dispatch_event(event));
    EXPECT_EQ(systems.get<ObservingSystem>().eventCalls, 1);
}

TEST(SystemRegistry, DispatchEventStopsAtTheFirstSystemThatConsumesIt)
{
    SystemRegistry systems;
    systems.add<ConsumingSystem>();
    systems.add<ObservingSystem>(); // registered second, must never be reached

    Event event;
    event.type = Event::KeyPressed;

    EXPECT_TRUE(systems.dispatch_event(event));
    EXPECT_EQ(systems.get<ConsumingSystem>().eventCalls, 1);
    EXPECT_EQ(systems.get<ObservingSystem>().eventCalls, 0);
}
