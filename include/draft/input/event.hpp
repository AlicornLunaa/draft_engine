#pragma once

#include <cstdint>

#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"

namespace Draft {
	class Event {
	public:
		struct SizeEvent {
			unsigned int width;
			unsigned int height;
		};

		struct KeyEvent {
			Key code;
			bool alt;
			bool control;
			bool shift;
			bool system;
		};

		struct TextEvent {
			uint32_t unicode;
		};

		struct MouseMoveEvent {
			int x;
			int y;
		};

		struct MouseButtonEvent {
			Button button;
			int x;
			int y;
		};

		struct MouseWheelEvent {
			int delta;
			int x;
			int y;
		};

		struct MouseWheelScrollEvent {
			// Mouse::Wheel wheel;
			float delta;
			int x;
			int y;
		};

		enum EventType {
			Closed,
			Resized,
			LostFocus,
			GainedFocus,
			TextEntered,
			KeyPressed,
			KeyReleased,
			MouseWheelMoved,
			MouseWheelScrolled,
			MouseButtonPressed,
			MouseButtonReleased,
			MouseMoved,
			MouseEntered,
			MouseLeft,

			Count
		};

		// Member data
		EventType type;

		union {
			SizeEvent size;
			KeyEvent key;
			TextEvent text;
			MouseMoveEvent mouseMove;
			MouseButtonEvent mouseButton;
			MouseWheelEvent mouseWheel;
			MouseWheelScrollEvent mouseWheelScroll;
		};
	};
};