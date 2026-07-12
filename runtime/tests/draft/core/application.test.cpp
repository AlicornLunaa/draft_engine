#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/core/application.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include <memory>
#include <vector>

using namespace Draft;

namespace {
    // Renderer is abstract (render_frame is pure virtual). A trivial concrete subclass is
    // needed to construct one at all, matching every other pipeline test's TestRenderer. Counts
    // resize() calls so a test can distinguish "resized exactly once" from "resized every step".
    class CountingRenderer : public Renderer {
    public:
        using Renderer::Renderer;
        int resizeCalls = 0;
        void render_frame(Time, SystemRegistry&) override {}
        void resize(const Vector2u& size) override {
            resizeCalls++;
            Renderer::resize(size);
        }
    };

    struct CountingSystem : AbstractSystem {
        int updateCalls = 0;
        int renderCalls = 0;
        int attachCalls = 0;
        int detachCalls = 0;
        int eventCalls = 0;
        bool consumeEvents = false;

        void update(Time) override { updateCalls++; }
        void render(Time, RenderLayer) override { renderCalls++; }
        void on_attach() override { attachCalls++; }
        void on_detach() override { detachCalls++; }
        bool on_event(const Event&) override { eventCalls++; return consumeEvents; }
    };
}

class ApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }
};

TEST_F(ApplicationTest, ConstructionSucceedsWithNoActiveSceneAndARealDefaultRenderer)
{
    Application app("application_test", 320, 320);

    EXPECT_EQ(app.get_scene(), nullptr);
    ASSERT_NE(app.get_renderer(), nullptr);
    EXPECT_EQ(app.get_renderer()->get_size().x, 320u);
    EXPECT_EQ(app.get_renderer()->get_size().y, 320u);
}

TEST_F(ApplicationTest, StepTicksTheActiveSceneOnBothCadences)
{
    Application app("application_test", 64, 64);
    Scene scene;
    scene.get_systems().add<CountingSystem>();

    // Small enough that a real elapsed step() call reliably exceeds it at least once, so the
    // fixed-timestep accumulator actually fires update(). A real frame's dt (however brief) is
    // never exactly zero.
    app.timeStep = Time::seconds(1.0 / 100000.0);
    app.set_scene(&scene);

    // The first step() after set_scene() applies the deferred swap and resets the accumulator/
    // deltaTime to avoid a large dt spike (see scene_change()) that reset zeroes this same
    // frame's deltaTime, so update() correctly doesn't fire until the step() after.
    app.step();
    app.step();

    EXPECT_GE(scene.get_systems().get<CountingSystem>().updateCalls, 1);
    EXPECT_EQ(scene.get_systems().get<CountingSystem>().renderCalls, 2);
}

TEST_F(ApplicationTest, SetSceneFiresDetachOnTheOldSceneAndAttachOnTheNewOneInOrder)
{
    Application app("application_test", 64, 64);
    Scene sceneA;
    Scene sceneB;
    sceneA.get_systems().add<CountingSystem>();
    sceneB.get_systems().add<CountingSystem>();

    app.set_scene(&sceneA);
    app.step(); // sceneA becomes active and attach fires, nothing to detach yet

    EXPECT_EQ(sceneA.get_systems().get<CountingSystem>().attachCalls, 1);
    EXPECT_EQ(sceneA.get_systems().get<CountingSystem>().detachCalls, 0);

    app.set_scene(&sceneB);
    app.step(); // sceneA detaches, sceneB attaches

    EXPECT_EQ(sceneA.get_systems().get<CountingSystem>().detachCalls, 1);
    EXPECT_EQ(sceneB.get_systems().get<CountingSystem>().attachCalls, 1);
}

TEST_F(ApplicationTest, SetSceneResetsTheAccumulatorAndDeltaTimeOnTheNextStep)
{
    Application app("application_test", 64, 64);
    Scene scene;
    scene.get_systems().add<CountingSystem>();

    app.set_scene(&scene);
    app.step();

    // A second scene swap must reset deltaTime rather than letting whatever elapsed between
    // construction and this point carry over as a spurious spike.
    Scene otherScene;
    app.set_scene(&otherScene);
    app.step();

    EXPECT_GE(app.deltaTime.as_seconds(), 0.f);
}

TEST_F(ApplicationTest, EventCallbackFiresForRealWindowAndKeyboardCallbacks)
{
    Application app("application_test", 64, 64);

    std::vector<Event::EventType> seen;
    app.eventCallback = [&](const Event& event){
        seen.push_back(event.type);
        return false;
    };

    app.keyboard.keyCallback(Keyboard::SPACE, 1, 0); // Action::PRESS
    app.window.closeCallback();

    ASSERT_EQ(seen.size(), 2u);
    EXPECT_EQ(seen[0], Event::KeyPressed);
    EXPECT_EQ(seen[1], Event::Closed);
}

TEST_F(ApplicationTest, EventReachesActiveSceneSystemWhenEventCallbackDoesNotConsumeIt)
{
    Application app("application_test", 64, 64);
    Scene scene;
    CountingSystem& system = scene.get_systems().add<CountingSystem>();
    app.set_scene(&scene);
    app.step(); // apply the pending scene swap

    app.keyboard.keyCallback(Keyboard::SPACE, 1, 0);

    EXPECT_EQ(system.eventCalls, 1);
}

TEST_F(ApplicationTest, EventCallbackConsumingAnEventPreemptsTheActiveScene)
{
    Application app("application_test", 64, 64);
    Scene scene;
    CountingSystem& system = scene.get_systems().add<CountingSystem>();
    app.set_scene(&scene);
    app.step();

    app.eventCallback = [](const Event&){ return true; };
    app.keyboard.keyCallback(Keyboard::SPACE, 1, 0);

    EXPECT_EQ(system.eventCalls, 0);
}

TEST_F(ApplicationTest, ResizeCallsRendererResizeExactlyOnceNotEveryStep)
{
    Application app("application_test", 64, 64);

    auto renderer = std::make_unique<CountingRenderer>(Vector2u{64, 64});
    CountingRenderer* rendererPtr = renderer.get();
    app.set_renderer(std::move(renderer));
    app.step(); // applies the pending renderer swap

    // trigger_resize() re-reads the real window.get_frame_size() rather than trusting the
    // callback's own ints, so this only needs to set pendingResize
    app.window.frameSizeCallback(128, 96);
    app.step(); // applies the pending resize, resize() should fire exactly once here

    EXPECT_EQ(rendererPtr->resizeCalls, 1);

    // Regression pin for the old engine's unconditional per-frame resize call
    app.step();
    app.step();

    EXPECT_EQ(rendererPtr->resizeCalls, 1);
}
