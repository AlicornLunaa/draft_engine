#include <gtest/gtest.h>
#include "draft/audio/music.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "wav_test_helper.hpp"
#include <filesystem>

using namespace Draft;
using namespace Draft::Testing;

namespace {
    // Music::load(path) streams straight from disk (sf::Music::openFromFile), so most of these
    // tests still need a real file on disk rather than in-memory bytes.
    class MusicTest : public ::testing::Test {
    protected:
        HostFileSystem fs;
        std::filesystem::path path = "music_test.wav";

        void SetUp() override {
            fs.write_bytes(path, make_wav_bytes(44100, 44100));
        }

        void TearDown() override {
            fs.remove(path);
        }
    };
}

TEST_F(MusicTest, LoadReadsSampleRateAndChannelCount)
{
    Music music(fs.open(path));

    EXPECT_EQ(music.get_sample_rate(), 44100u);
    EXPECT_EQ(music.get_channel_count(), 1u);
    EXPECT_EQ(music.get_duration().as_milliseconds(), 1000);
}

TEST_F(MusicTest, PropertiesRoundTrip)
{
    Music music(fs.open(path));
    music.set_pitch(1.25f);
    music.set_volume(60.f);
    music.set_loop(true);
    music.set_relative(true);
    music.set_min_distance(3.f);
    music.set_attenuation(0.25f);
    music.set_position({1.f, 2.f, 3.f});

    EXPECT_FLOAT_EQ(music.get_pitch(), 1.25f);
    EXPECT_FLOAT_EQ(music.get_volume(), 60.f);
    EXPECT_TRUE(music.get_loop());
    EXPECT_TRUE(music.is_relative());
    EXPECT_FLOAT_EQ(music.get_min_distance(), 3.f);
    EXPECT_FLOAT_EQ(music.get_attenuation(), 0.25f);
    EXPECT_FLOAT_EQ(music.get_position().x, 1.f);
}

TEST_F(MusicTest, LoopPointsRoundTrip)
{
    Music music(fs.open(path));
    music.set_loop_points(Time::milliseconds(100), Time::milliseconds(400));

    auto [start, end] = music.get_loop_points();
    EXPECT_EQ(start.as_milliseconds(), 100);
    EXPECT_EQ(end.as_milliseconds(), 400);
}

TEST_F(MusicTest, PlayPauseStopDoNotThrow)
{
    Music music(fs.open(path));

    ASSERT_NO_THROW(music.play());
    ASSERT_NO_THROW(music.pause());
    ASSERT_NO_THROW(music.stop());
}

TEST_F(MusicTest, LoadFileHandleOverloadReplacesAnExistingTrack)
{
    Music music(fs.open(path));

    fs.write_bytes(path, make_wav_bytes(22050, 22050));
    music.load(fs.open(path));

    EXPECT_EQ(music.get_sample_rate(), 22050u);
    EXPECT_EQ(music.get_duration().as_milliseconds(), 1000);
}

TEST_F(MusicTest, FileHandleLoadedMusicPlaysWithoutThrowing)
{
    Music music(fs.open(path));

    ASSERT_NO_THROW(music.play());
    ASSERT_NO_THROW(music.stop());
}
