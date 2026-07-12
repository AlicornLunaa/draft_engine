#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace Draft::Testing {
    /**
     * @brief Builds a minimal, valid mono 16-bit PCM WAV file in memory, so audio tests don't
     * depend on a fixture asset on disk. just enough of the RIFF/WAVE format for SFML's own
     * decoder to accept it.
     */
    inline std::vector<std::byte> make_wav_bytes(uint32_t sampleRate = 44100, uint32_t sampleCount = 4410){
        constexpr uint16_t channels = 1;
        constexpr uint16_t bitsPerSample = 16;
        const uint32_t byteRate = sampleRate * channels * bitsPerSample / 8;
        const uint16_t blockAlign = channels * bitsPerSample / 8;
        const uint32_t dataSize = sampleCount * blockAlign;
        const uint32_t riffSize = 36 + dataSize;

        std::vector<std::byte> bytes(44 + dataSize);
        unsigned char* out = reinterpret_cast<unsigned char*>(bytes.data());

        auto put_u32 = [&](size_t offset, uint32_t value){ std::memcpy(out + offset, &value, sizeof(value)); };
        auto put_u16 = [&](size_t offset, uint16_t value){ std::memcpy(out + offset, &value, sizeof(value)); };

        std::memcpy(out + 0, "RIFF", 4);
        put_u32(4, riffSize);
        std::memcpy(out + 8, "WAVE", 4);
        std::memcpy(out + 12, "fmt ", 4);
        put_u32(16, 16); // fmt chunk size
        put_u16(20, 1); // PCM
        put_u16(22, channels);
        put_u32(24, sampleRate);
        put_u32(28, byteRate);
        put_u16(32, blockAlign);
        put_u16(34, bitsPerSample);
        std::memcpy(out + 36, "data", 4);
        put_u32(40, dataSize);

        // A simple sine-ish ramp so the samples aren't all zero
        int16_t* samples = reinterpret_cast<int16_t*>(out + 44);
        for(uint32_t i = 0; i < sampleCount; ++i)
            samples[i] = static_cast<int16_t>((i % 256) * 128);

        return bytes;
    }
}
