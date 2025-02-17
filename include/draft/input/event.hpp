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
			int mods;
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
			int mods;
		};

		struct MouseWheelScrollEvent {
			double x;
			double y;
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
			MouseWheelScrollEvent mouseWheelScroll;
		};
	};
	
	typedef std::function<void(Event event)> EventCallback;
};