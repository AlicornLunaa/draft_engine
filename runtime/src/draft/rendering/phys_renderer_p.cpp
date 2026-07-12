#include "phys_renderer_p.hpp"
#include "box2d/b2_draw.h"
#include "box2d/b2_math.h"
#include "draft/rendering/batching/shape_point.hpp"

#include <cassert>
#include <vector>

namespace Draft {
    PhysicsDebugRender::PhysicsDebugRender() {
        SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_centerOfMassBit);
    }

    void PhysicsDebugRender::begin(Renderer& renderer, const Matrix4& m){
        assert(!m_rendererThisPass && "PhysicsDebugRender::begin(...) called after itself or before PhysicsDebugRender::finish()");
        m_rendererThisPass = &renderer;
        renderer.shape.set_proj_matrix(m);
        renderer.shape.set_render_type(ShapeRenderType::LINE);
    }

    void PhysicsDebugRender::finish(){
        m_rendererThisPass = nullptr;
    }

    /// Draw a closed polygon provided in CCW order.
    void PhysicsDebugRender::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color){
        std::vector<Vector2f> arr(vertexCount);

        for(size_t i = 0; i < vertexCount; i++){
            arr[i] = { vertices[i].x, vertices[i].y };
        }

        assert(m_rendererThisPass && "PhysicsDebugRender::DrawPolygon(...) called before PhysicsDebugRender::begin(...)");
        m_rendererThisPass->shape.set_color({ color.r, color.g, color.b, color.a });
        m_rendererThisPass->shape.draw_polygon(arr);
    }

    /// Draw a solid closed polygon provided in CCW order.
    void PhysicsDebugRender::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color){
        std::vector<Vector2f> arr(vertexCount);

        for(size_t i = 0; i < vertexCount; i++){
            arr[i] = { vertices[i].x, vertices[i].y };
        }

        assert(m_rendererThisPass && "PhysicsDebugRender::DrawSolidPolygon(...) called before PhysicsDebugRender::begin(...)");
        m_rendererThisPass->shape.set_color({ color.r, color.g, color.b, color.a });
        m_rendererThisPass->shape.draw_polygon(arr);
    }

    /// Draw a circle.
    void PhysicsDebugRender::DrawCircle(const b2Vec2& center, float radius, const b2Color& color){
        assert(m_rendererThisPass && "PhysicsDebugRender::DrawCircle(...) called before PhysicsDebugRender::begin(...)");
        m_rendererThisPass->shape.set_color({ color.r, color.g, color.b, color.a });
        m_rendererThisPass->shape.draw_circle({ center.x, center.y }, radius, 0.f, 40);
    }

    /// Draw a solid circle.
    void PhysicsDebugRender::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color){
        assert(m_rendererThisPass && "PhysicsDebugRender::DrawSolidCircle(...) called before PhysicsDebugRender::begin(...)");
        m_rendererThisPass->shape.set_color({ color.r, color.g, color.b, color.a });
        m_rendererThisPass->shape.draw_circle({ center.x, center.y }, radius, 0.f, 40);
    }

    /// Draw a line segment.
    void PhysicsDebugRender::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color){
        assert(m_rendererThisPass && "PhysicsDebugRender::DrawSegment(...) called before PhysicsDebugRender::begin(...)");
        m_rendererThisPass->shape.set_color({ color.r, color.g, color.b, color.a });
        m_rendererThisPass->shape.draw_line({ p1.x, p1.y }, { p2.x, p2.y });
    }

    /// Draw a transform. Choose your own length scale.
    /// @param xf a transform.
    void PhysicsDebugRender::DrawTransform(const b2Transform& xf){
        b2Vec2 p = xf.p;
        b2Vec2 px = p + (0.5f * xf.q.GetXAxis());
        b2Vec2 py = p + (0.5f * xf.q.GetYAxis());

        assert(m_rendererThisPass && "PhysicsDebugRender::DrawTransform(...) called before PhysicsDebugRender::begin(...)");
        m_rendererThisPass->shape.set_color({ 1, 0, 0, 1 });
        m_rendererThisPass->shape.draw_line({ p.x, p.y }, { px.x, px.y });
        m_rendererThisPass->shape.draw_line({ p.x, p.y }, { py.x, py.y });
    }

    /// Draw a point.
    void PhysicsDebugRender::DrawPoint(const b2Vec2& p, float size, const b2Color& color){
        assert(m_rendererThisPass && "PhysicsDebugRender::DrawPoint(...) called before PhysicsDebugRender::begin(...)");
        m_rendererThisPass->shape.set_render_type(ShapeRenderType::FILL);
        m_rendererThisPass->shape.set_color({ color.r, color.g, color.b, color.a });
        m_rendererThisPass->shape.draw_circle({ p.x, p.y }, size * 0.003f, 0.f, 40);
        m_rendererThisPass->shape.set_render_type(ShapeRenderType::LINE);
    }
};
