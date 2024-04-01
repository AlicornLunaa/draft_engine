// #include "draft/rendering/render_window.hpp"
// #include "SFML/Graphics/RenderWindow.hpp"
// #include "SFML/Window/VideoMode.hpp"
// #include "draft/math/vector2_p.hpp"
// #include <memory>

// using namespace std;

// namespace Draft {
//     // Definitions
//     RenderWindow::RenderWindow() : ptr(std::make_unique<sf::RenderWindow>()) {}
//     RenderWindow::RenderWindow(unsigned int width, unsigned int height, const string& title) : ptr(std::make_unique<sf::RenderWindow>(sf::VideoMode(width, height), title)) {}
//     RenderWindow::~RenderWindow(){}

//     // Functions
//     bool RenderWindow::is_srgb() const { return ptr->isSrgb(); }
//     bool RenderWindow::is_open() const { return ptr->isOpen(); }

//     bool RenderWindow::set_active(bool active){ return ptr->setActive(active); }
//     void RenderWindow::set_vertical_sync(bool enabled){ ptr->setVerticalSyncEnabled(enabled); }
//     void RenderWindow::set_framerate_limit(unsigned int limit){ ptr->setFramerateLimit(limit); }

//     bool RenderWindow::poll_event(sf::Event& event){ return ptr->pollEvent(event); }
//     bool RenderWindow::wait_event(sf::Event& event){ return ptr->waitEvent(event); }
//     void RenderWindow::clear(const sf::Color& color){ ptr->clear(color); };
//     void RenderWindow::display(){ ptr->display(); }
//     void RenderWindow::close(){ ptr->close(); }

//     void RenderWindow::draw(const sf::Drawable& drawable, const sf::RenderStates& states){ ptr->draw(drawable, states); }
//     void RenderWindow::draw(const sf::Vertex* vertices, size_t vertexCount, sf::PrimitiveType type, const sf::RenderStates& states){ ptr->draw(vertices, vertexCount, type, states); }
//     void RenderWindow::draw(const sf::VertexBuffer& buffer, const sf::RenderStates& states){ ptr->draw(buffer, states); }
//     void RenderWindow::draw(const sf::VertexBuffer& buffer, size_t firstVertex, size_t count, const sf::RenderStates& states){ ptr->draw(buffer, firstVertex, count, states); }
//     void RenderWindow::push_gl_states(){ ptr->pushGLStates(); }
//     void RenderWindow::pop_gl_states(){ ptr->popGLStates(); }
//     void RenderWindow::reset_gl_states(){ ptr->resetGLStates(); }

//     void RenderWindow::request_focus(){ ptr->requestFocus(); }
//     bool RenderWindow::has_focus(){ return ptr->hasFocus(); }

//     Vector2i RenderWindow::get_position() const { return sf_to_vector(ptr->getPosition()); };
//     Vector2u RenderWindow::get_size() const { return sf_to_vector(ptr->getSize()); };
//     sf::IntRect RenderWindow::get_viewport(const Camera& camera) const { return ptr->getViewport(camera); };
//     const Camera& RenderWindow::get_view() const { return ptr->getView(); };
//     const Camera& RenderWindow::get_default_view() const { return ptr->getDefaultView(); }
//     void RenderWindow::set_position(const Vector2i& v){ ptr->setPosition(vector_to_sf(v)); }
//     void RenderWindow::set_size(const Vector2u& v){ ptr->setSize(vector_to_sf(v)); };
//     void RenderWindow::set_title(const std::string& title){ ptr->setTitle(title); }
//     void RenderWindow::set_icon(unsigned int width, unsigned int height, const uint8_t* pixels){ ptr->setIcon(width, height, pixels); }
//     void RenderWindow::set_visible(bool visible){ ptr->setVisible(visible); }
//     void RenderWindow::set_mouse_visible(bool visible){ ptr->setMouseCursorVisible(visible); }
//     void RenderWindow::set_mouse_cursor_grabbed(bool grabbed){ ptr->setMouseCursorGrabbed(grabbed); }
//     void RenderWindow::set_mouse_cursor(const sf::Cursor& cursor){ ptr->setMouseCursor(cursor); }
//     void RenderWindow::set_key_repeat(bool enabled){ ptr->setKeyRepeatEnabled(enabled); }
//     void RenderWindow::set_view(const Camera& camera){ ptr->setView(camera); }

//     Vector2f RenderWindow::map_pixel_to_coords(const Vector2i& pixel) const { return sf_to_vector(ptr->mapPixelToCoords(vector_to_sf(pixel))); };
//     Vector2f RenderWindow::map_pixel_to_coords(const Vector2i& pixel, const Camera& camera) const { return sf_to_vector(ptr->mapPixelToCoords(vector_to_sf(pixel), camera)); };
//     Vector2i RenderWindow::map_coords_to_pixel(const Vector2f& point) const { return sf_to_vector(ptr->mapCoordsToPixel(vector_to_sf(point))); };
//     Vector2i RenderWindow::map_coords_to_pixel(const Vector2f& point, const Camera& camera) const { return sf_to_vector(ptr->mapCoordsToPixel(vector_to_sf(point), camera)); };

//     sf::RenderWindow& RenderWindow::get_impl(){ return *ptr; }
// };