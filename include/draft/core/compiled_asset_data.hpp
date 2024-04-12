#pragma once

// This class simply stores static data that will never change.
// Stuff like a default font, missing texture, etc

namespace Draft {
    struct StaticImageData {
        int width;
        int height;
        int nrChannels;
        unsigned char data[];
    };

    namespace StaticData {
        static const StaticImageData BASE {
            
        };
    };
};