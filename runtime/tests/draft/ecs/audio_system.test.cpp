#include <gtest/gtest.h>
#include "draft/ecs/audio_system.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/components/audio_components.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "../audio/wav_test_helper.hpp"
#include <filesystem>
#include <memory>

using namespace Draft;
using namespace Draft::Testing;

namespace {
    class AudioSystemTest : public ::testing::Test {
    protected:
        HostFileSystem fs;
        std::filesystem::path musicPath = "audio_system_test.wav";

        void SetUp() override {
            fs.write_bytes(musicPath, make_wav_bytes());
        }

        void TearDown() override {
            fs.remove(musicPath);
        }
    };
}

TEST_F(AudioSystemTest, RenderSyncsListenerPositionFromTransform)
{
    // Listener::apply() pushes straight to SFML's global listener state, which only exists
    // while at least one SFML sound source (Sound/Music) is alive, keep one around so this
    // test exercises a real, live audio device instead of a torn-down one.
    SoundBuffer keepAliveBuffer(make_wav_bytes());
    Sound keepAliveSound(keepAliveBuffer);

    Scene scene;
    scene.get_systems().add<AudioSystem>(scene.get_registry());

    Entity listenerEntity = scene.create_entity();
    listenerEntity.add_component<TransformComponent>(TransformComponent{{2.f, 3.f}, 0.f});
    ListenerComponent& listenerComp = listenerEntity.add_component<ListenerComponent>();

    scene.render(Time::seconds(0));

    EXPECT_FLOAT_EQ(listenerComp.listener.get_position().x, 2.f);
    EXPECT_FLOAT_EQ(listenerComp.listener.get_position().y, 3.f);
}

TEST_F(AudioSystemTest, RenderSyncsSoundPositionFromTransform)
{
    Scene scene;
    scene.get_systems().add<AudioSystem>(scene.get_registry());

    SoundBuffer buffer(make_wav_bytes());
    Sound sound(buffer);

    Entity soundEntity = scene.create_entity();
    soundEntity.add_component<TransformComponent>(TransformComponent{{5.f, 7.f}, 0.f});
    SoundComponent& soundComp = soundEntity.add_component<SoundComponent>(sound);

    scene.render(Time::seconds(0));

    EXPECT_FLOAT_EQ(soundComp.sound.get_position().x, 5.f);
    EXPECT_FLOAT_EQ(soundComp.sound.get_position().y, 7.f);
}

TEST_F(AudioSystemTest, RenderSyncsMusicPositionFromTransform)
{
    Scene scene;
    scene.get_systems().add<AudioSystem>(scene.get_registry());

    auto music = std::make_shared<Music>(fs.open(musicPath));
    Resource<Music> track(std::make_shared<AssetSlot<Music>>(std::move(music)));

    Entity musicEntity = scene.create_entity();
    musicEntity.add_component<TransformComponent>(TransformComponent{{9.f, 11.f}, 0.f});
    MusicComponent& musicComp = musicEntity.add_component<MusicComponent>(track);

    scene.render(Time::seconds(0));

    EXPECT_FLOAT_EQ(musicComp.track->get_position().x, 9.f);
    EXPECT_FLOAT_EQ(musicComp.track->get_position().y, 11.f);
}

TEST_F(AudioSystemTest, RenderWithNoAudioEntitiesDoesNotThrow)
{
    Scene scene;
    scene.get_systems().add<AudioSystem>(scene.get_registry());

    ASSERT_NO_THROW(scene.render(Time::seconds(0)));
}
