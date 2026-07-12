#include <gtest/gtest.h>
#include "draft/audio/sound_buffer.hpp"
#include "draft/util/files/virtual_file_system.hpp"
#include "wav_test_helper.hpp"

using namespace Draft;
using namespace Draft::Testing;

TEST(SoundBuffer, LoadsFromRawBytes)
{
    SoundBuffer buffer(make_wav_bytes(44100, 4410));

    EXPECT_EQ(buffer.get_sample_rate(), 44100u);
    EXPECT_EQ(buffer.get_channel_count(), 1u);
    EXPECT_EQ(buffer.get_sample_count(), 4410u);
    EXPECT_NE(buffer.get_samples(), nullptr);
    EXPECT_EQ(buffer.get_duration().as_milliseconds(), 100);
}

TEST(SoundBuffer, LoadsFromFileHandle)
{
    VirtualFileSystem fs;
    auto handle = fs.open("sound_buffer_test.wav");
    fs.write_bytes("sound_buffer_test.wav", make_wav_bytes(22050, 2205));

    SoundBuffer buffer(handle);

    EXPECT_EQ(buffer.get_sample_rate(), 22050u);
    EXPECT_EQ(buffer.get_sample_count(), 2205u);

    fs.remove("sound_buffer_test.wav");
}

TEST(SoundBuffer, CopyConstructorDuplicatesTheDecodedBuffer)
{
    SoundBuffer original(make_wav_bytes(44100, 4410));
    SoundBuffer copy(original);

    EXPECT_EQ(copy.get_sample_rate(), original.get_sample_rate());
    EXPECT_EQ(copy.get_sample_count(), original.get_sample_count());
}
