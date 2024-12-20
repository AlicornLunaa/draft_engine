#include "draft/util/color.hpp"
#include <sstream>
#include <iomanip>
#include <ios>

// Static data
const Color Color::WHITE = Color("#F0F0F0");
const Color Color::ORANGE = Color("#F2B233");
const Color Color::MAGENTA = Color("#E57FD8");
const Color Color::LIGHT_BLUE = Color("#99B2F2");
const Color Color::YELLOW = Color("#DEDE6C");
const Color Color::LIME = Color("#7FCC19");
const Color Color::PINK = Color("#F2B2CC");
const Color Color::GRAY = Color("#4C4C4C");
const Color Color::LIGHT_GRAY = Color("#999999");
const Color Color::CYAN = Color("#4C99B2");
const Color Color::PURPLE = Color("#B266E5");
const Color Color::BLUE = Color("#3366CC");
const Color Color::BROWN = Color("#7F664C");
const Color Color::GREEN = Color("#57A64E");
const Color Color::RED = Color("#CC4C4C");
const Color Color::BLACK = Color("#111111");

// Constructors
Color::Color(std::string hex){
    // Converts the hex to RGBA float. First step is to remove the first # if it exists
    if(hex[0] == '#')
        hex = hex.substr(1);

    // Slight error handling
    assert((hex.size() == 3 || hex.size() == 4 || hex.size() == 6 || hex.size() == 8) && "Hex value not valid, syntax: #FFFFFF or #FFF");
    uint r, g, b, a = 255;

    // Now decide whether its 3 hex values or 6
    if(hex.size() == 3){
        std::stringstream(hex.substr(0, 1)) >> std::hex >> r;
        std::stringstream(hex.substr(1, 1)) >> std::hex >> g;
        std::stringstream(hex.substr(2, 1)) >> std::hex >> b;
        r *= 17;
        g *= 17;
        b *= 17;
    } else if(hex.size() == 4){
        std::stringstream(hex.substr(0, 1) + hex.substr(0, 1)) >> std::hex >> r;
        std::stringstream(hex.substr(1, 1) + hex.substr(1, 1)) >> std::hex >> g;
        std::stringstream(hex.substr(2, 1) + hex.substr(2, 1)) >> std::hex >> b;
        std::stringstream(hex.substr(3, 1) + hex.substr(3, 1)) >> std::hex >> a;
        r *= 17;
        g *= 17;
        b *= 17;
        a *= 17;
    } else if(hex.size() == 6){
        std::stringstream(hex.substr(0, 2)) >> std::hex >> r;
        std::stringstream(hex.substr(2, 2)) >> std::hex >> g;
        std::stringstream(hex.substr(4, 2)) >> std::hex >> b;
    } else if(hex.size() == 8){
        std::stringstream(hex.substr(0, 2)) >> std::hex >> r;
        std::stringstream(hex.substr(2, 2)) >> std::hex >> g;
        std::stringstream(hex.substr(4, 2)) >> std::hex >> b;
        std::stringstream(hex.substr(6, 2)) >> std::hex >> a;
    }

    x = r / 255.f;
    y = g / 255.f;
    z = b / 255.f;
    w = a / 255.f;
}

// Functions
std::string Color::to_hex(bool alpha) const {
    // Converts the RGBA float to hex
    uint r1 = static_cast<uint>(r * 255.0f);
    uint g1 = static_cast<uint>(g * 255.0f);
    uint b1 = static_cast<uint>(b * 255.0f);
    uint a1 = static_cast<uint>(a * 255.0f);

    std::ostringstream hexColor;
    hexColor << "#"
             << std::setw(2) << std::setfill('0') << std::hex << r1
             << std::setw(2) << std::setfill('0') << std::hex << g1
             << std::setw(2) << std::setfill('0') << std::hex << b1;

    return hexColor.str();
}