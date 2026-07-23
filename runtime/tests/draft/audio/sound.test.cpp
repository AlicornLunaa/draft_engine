#include <gtest/gtest.h>
#include "draft/asset/resource.hpp"
#include "draft/audio/sound.hpp"
#include "wav_test_helper.hpp"
#include <memory>

using namespace Draft;
using namespace Draft::Testing;

namespace {
    Resource<SoundBuffer> make_buffer_resource(){
        auto buffer = std::make_shared<SoundBuffer>(make_wav_bytes());
        return Resource<SoundBuffer>(std::make_shared<AssetSlot<SoundBuffer>>(std::move(buffer)));
    }
}

TEST(Sound, DefaultConstructedHasNoBuffer)
{
    Sound sound;
    EXPECT_FALSE(sound.get_buffer().is_valid());
}

TEST(Sound, SetBufferTracksThePassedBuffer)
{
    Resource<SoundBuffer> buffer = make_buffer_resource();
    Sound sound;
    sound.set_buffer(buffer);

    EXPECT_EQ(sound.get_buffer().get(), buffer.get());
}

TEST(Sound, ConstructingFromABufferBindsItImmediately)
{
    Resource<SoundBuffer> buffer = make_buffer_resource();
    Sound sound(buffer);

    EXPECT_EQ(sound.get_buffer().get(), buffer.get());
}

TEST(Sound, CopyConstructorPreservesTheBufferPointer)
{
    Resource<SoundBuffer> buffer = make_buffer_resource();
    Sound original(buffer);
    Sound copy(original);

    EXPECT_EQ(copy.get_buffer().get(), buffer.get());
}

TEST(Sound, CopyAssignmentPreservesTheBufferPointer)
{
    Resource<SoundBuffer> buffer = make_buffer_resource();
    Sound original(buffer);
    Sound copy;
    copy = original;

    EXPECT_EQ(copy.get_buffer().get(), buffer.get());
}

TEST(Sound, PropertiesRoundTrip)
{
    Sound sound;
    sound.set_loop(true);
    sound.set_pitch(1.5f);
    sound.set_volume(50.f);
    sound.set_position({1.f, 2.f, 3.f});
    sound.set_min_distance(5.f);
    sound.set_attenuation(0.5f);
    sound.set_relative(true);

    EXPECT_TRUE(sound.get_loop());
    EXPECT_FLOAT_EQ(sound.get_pitch(), 1.5f);
    EXPECT_FLOAT_EQ(sound.get_volume(), 50.f);
    EXPECT_FLOAT_EQ(sound.get_position().x, 1.f);
    EXPECT_FLOAT_EQ(sound.get_position().y, 2.f);
    EXPECT_FLOAT_EQ(sound.get_position().z, 3.f);
    EXPECT_FLOAT_EQ(sound.get_min_distance(), 5.f);
    EXPECT_FLOAT_EQ(sound.get_attenuation(), 0.5f);
    EXPECT_TRUE(sound.is_relative());
}

TEST(Sound, PlayPauseStopDoNotThrow)
{
    Resource<SoundBuffer> buffer = make_buffer_resource();
    Sound sound(buffer);

    ASSERT_NO_THROW(sound.play());
    ASSERT_NO_THROW(sound.pause());
    ASSERT_NO_THROW(sound.stop());
}

TEST(Sound, ResetBufferDoesNotThrow)
{
    Resource<SoundBuffer> buffer = make_buffer_resource();
    Sound sound(buffer);

    ASSERT_NO_THROW(sound.reset_buffer());
}
