#pragma once

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Drawable.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/Event.hpp"
#include "draft/math/vector2.hpp"
#include "draft/rendering/camera.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace sf { class RenderWindow; };

namespace Draft {
    class RenderWindow {
    public:
        // Constructors
        RenderWindow();
        RenderWindow(unsigned int width, unsigned int height, const std::string& title);
        RenderWindow(const RenderWindow& other) = delete;
        ~RenderWindow();

        // Functions
        bool is_srgb() const;
        bool is_open() const;

        bool set_active(bool active = true);
        void set_vertical_sync(bool enabled);
        void set_framerate_limit(unsigned int limit);

        const sf::ContextSettings& get_settings() const;
        bool poll_event(sf::Event& event);
        bool wait_event(sf::Event& event);
        void clear(const sf::Color& color = sf::Color(0, 0, 0, 255));
        void display();
        void close();

        void draw(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates::Default);
        void draw(const sf::Vertex* vertices, size_t vertexCount, sf::PrimitiveType type, const sf::RenderStates& states = sf::RenderStates::Default);
        void draw(const sf::VertexBuffer& buffer, const sf::RenderStates& states = sf::RenderStates::Default);
        void draw(const sf::VertexBuffer& buffer, size_t firstVertex, size_t count, const sf::RenderStates& states = sf::RenderStates::Default);
        void push_gl_states();
        void pop_gl_states();
        void reset_gl_states();

        void request_focus();
        bool has_focus();

        Vector2i get_position() const;
        Vector2u get_size() const;
        sf::IntRect get_viewport(const Camera& camera) const;
        const Camera& get_view() const;
        const Camera& get_default_view() const;
        void set_position(const Vector2i& v);
        void set_size(const Vector2u& v);
        void set_title(const std::string& title);
        void set_icon(unsigned int width, unsigned int height, const uint8_t* pixels);
        void set_visible(bool visible);
        void set_mouse_visible(bool visible);
        void set_mouse_cursor_grabbed(bool grabbed);
        void set_mouse_cursor(const sf::Cursor& cursor);
        void set_key_repeat(bool enabled);
        void set_view(const Camera& camera);

        Vector2f map_pixel_to_coords(const Vector2i& pixel) const;
        Vector2f map_pixel_to_coords(const Vector2i& pixel, const Camera& camera) const;
        Vector2i map_coords_to_pixel(const Vector2f& point) const;
        Vector2i map_coords_to_pixel(const Vector2f& point, const Camera& camera) const;

        sf::RenderWindow& get_impl(); // Internal use

    private:
        // pImpl
        std::unique_ptr<sf::RenderWindow> ptr;
    };
};