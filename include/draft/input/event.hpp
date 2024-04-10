#pragma once

#include <cstdint>
#include <functional>

namespace Draft {
	class Event {
	public:
		struct SizeEvent {
			unsigned int width;
			unsigned int height;
		};

		struct KeyEvent {
			int code;
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
			int button;
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
			KeyHold,
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
	
	typedef std::function<void(Event event)> EventCallback;
};