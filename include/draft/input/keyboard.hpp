#pragma once

#include "draft/input/event.hpp"

#include <unordered_map>
#include <vector>

namespace Draft {
    class RenderWindow;

    class Keyboard {
    public:
        // Enums
        enum Key {
            SPACE = 32,
            APOSTROPHE = 39,
            COMMA = 44,
            MINUS = 45,
            PERIOD = 46,
            SLASH = 47,
            ZERO = 48,
            ONE = 49,
            TWO = 50,
            THREE = 51,
            FOUR = 52,
            FIVE = 53,
            SIX = 54,
            SEVEN = 55,
            EIGHT = 56,
            NINE = 57,
            SEMICOLON = 59,
            EQUAL = 61,
            A = 65,
            B = 66,
            C = 67,
            D = 68,
            E = 69,
            F = 70,
            G = 71,
            H = 72,
            I = 73,
            J = 74,
            K = 75,
            L = 76,
            M = 77,
            N = 78,
            O = 79,
            P = 80,
            Q = 81,
            R = 82,
            S = 83,
            T = 84,
            U = 85,
            V = 86,
            W = 87,
            X = 88,
            Y = 89,
            Z = 90,
            LEFT_BRACKET = 91,
            BACKSLASH = 92,
            RIGHT_BRACKET = 93,
            GRAVE = 96,
            ESCAPE = 256,
            ENTER = 257,
            TAB = 258,
            BACKSPACE = 259,
            INSERT = 260,
            DELETE = 261,
            RIGHT = 262,
            LEFT = 263,
            DOWN = 264,
            UP = 265,
            PAGE_UP = 266,
            PAGE_DOWN = 267,
            HOME = 268,
            END = 269,
            CAPS_LOCK = 280,
            SCROLL_LOCK = 281,
            NUM_LOCK = 282,
            PRINT_SCREEN = 283,
            PAUSE = 284,
            F1 = 290,
            F2 = 291,
            F3 = 292,
            F4 = 293,
            F5 = 294,
            F6 = 295,
            F7 = 296,
            F8 = 297,
            F9 = 298,
            F10 = 299,
            F11 = 300,
            F12 = 301,
            F13 = 302,
            F14 = 303,
            F15 = 304,
            F16 = 305,
            F17 = 306,
            F18 = 307,
            F19 = 308,
            F20 = 309,
            F21 = 310,
            F22 = 311,
            F23 = 312,
            F24 = 313,
            F25 = 314,
            KP0 = 320,
            KP1 = 321,
            KP2 = 322,
            KP3 = 323,
            KP4 = 324,
            KP5 = 325,
            KP6 = 326,
            KP7 = 327,
            KP8 = 328,
            KP9 = 329,
            KP_DECIMAL = 330,
            KP_DIVIDE = 331,
            KP_MULTIPLY = 332,
            KP_SUBTRACT = 333,
            KP_ADD = 334,
            KP_ENTER = 335,
            KP_EQUAL = 336,
            LEFT_SHIFT = 340,
            LEFT_CONTROL = 341,
            LEFT_ALT = 342,
            LEFT_SUPER = 343,
            RIGHT_SHIFT = 344,
            RIGHT_CONTROL = 345,
            RIGHT_ALT = 346,
            RIGHT_SUPER = 347,
            MENU = 348,
            LAST_KEY = MENU
        };

        enum class Modifier {
            SHIFT = 0x0001,
            CTRL = 0x0002,
            ALT = 0x0004,
            SUPER = 0x0008,
            CAPS_LOCK = 0x0010,
            NUM_LOCK = 0x0020
        };

    private:
        // Impl for defining friends
        struct GLFWImpl;

        // Variables
        static std::unordered_map<void*, Keyboard*> windowKeyboardMap;
        mutable std::unordered_map<int, bool> lastPressedKeys;
        std::vector<EventCallback> callbacks;
        RenderWindow* window = nullptr;

    public:
        // Constructors
        Keyboard(RenderWindow& window);
        Keyboard(const Keyboard& other) = delete;
        ~Keyboard();

        // Functions
        /**
         * @brief Adds a callback to be executed when GLFW has a callback
         * @param func 
         */
        void add_callback(EventCallback func);

        /**
         * @brief Removes all the callbacks from the keyboard
         */
        void clear_callbacks();

        /**
         * @brief Checks if a key is currently pressed
         * @param key 
         * @return true 
         * @return false 
         */
        bool is_pressed(int key) const;

        /**
         * @brief Checks if a key has just been pressed last frame
         * @param key 
         * @return true 
         * @return false 
         */
        bool is_just_pressed(int key) const;

        /**
         * @brief Resets all the last key presses
         */
        void reset_keyboard_state();
    };
};