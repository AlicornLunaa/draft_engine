#pragma once

#include <cstdint>

namespace Draft {
    /**
     * @brief Which stage of a frame an AbstractSystem's render(dt, layer) call is for. A bitmask
     * so a system can participate in more than one stage (e.g. ImGuiSystem needs both Default,
     * to start its frame before anything else runs, and Overlay, to actually draw once the whole
     * pipeline has flushed).
     *
     * Default runs before Renderer::render_frame() at all (see Application::frame()), and always
     * runs even if no Renderer is set. It's the "ordinary per-frame work" stage, not tied to
     * rendering. Geometry/Interface/Overlay are driven by Renderer::render_frame() itself, in
     * that order, interleaved with its own pass pipeline (see DefaultRenderer::render_frame()).
     */
    enum class RenderLayer : std::uint32_t {
        None = 0,

        /// Non-rendering per-frame logic, always runs, renderer or not. Default for a system
        /// that doesn't override get_render_layers() at all, so this matches AbstractSystem's
        /// pre-layer render(dt) behavior exactly.
        Default = 1u << 0,

        /// World/3D/2D submission, flushed into GeometryPass's framebuffer.
        Geometry = 1u << 1,

        /// UI-batch submission (renderer.batch/renderer.shape content meant to draw on top of
        /// the composited scene), flushed by InterfacePass.
        Interface = 1u << 2,

        /// Raw GL draws that must land after the entire pipeline (Geometry -> Composite ->
        /// Interface) has finished, e.g. an immediate-mode UI toolkit's own draw call.
        Overlay = 1u << 3,

        All = 0xFFFFFFFFu
    };

    constexpr RenderLayer operator|(RenderLayer a, RenderLayer b){
        return static_cast<RenderLayer>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
    }

    constexpr RenderLayer operator&(RenderLayer a, RenderLayer b){
        return static_cast<RenderLayer>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
    }

    constexpr bool has_layer(RenderLayer mask, RenderLayer layer){
        return (mask & layer) != RenderLayer::None;
    }
}
