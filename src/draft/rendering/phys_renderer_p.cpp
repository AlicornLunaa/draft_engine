#include "draft/rendering/phys_renderer_p.hpp"
#include "box2d/b2_draw.h"
#include "box2d/b2_math.h"
#include "draft/rendering/batching/shape_batch.hpp"

namespace Draft {
    PhysicsDebugRender::PhysicsDebugRender(std::shared_ptr<Shader> shader) : batch(shader) {
        SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_centerOfMassBit);
    }

    void PhysicsDebugRender::begin(const Camera* camera){
        if(camera)
            batch.set_proj_matrix(camera->get_combined());

        batch.begin();
    }

    void PhysicsDebugRender::render(){
        batch.flush();
    }

    /// Draw a closed polygon provided in CCW order.
    void PhysicsDebugRender::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color){
        std::vector<Vector2f> arr(vertexCount);
        
        for(size_t i = 0; i < vertexCount; i++){
            arr[i] = { vertices[i].x, vertices[i].y };
        }

        batch.set_color({ color.r, color.g, color.b, color.a });
        batch.draw_polygon(arr);
    }

    /// Draw a solid closed polygon provided in CCW order.
    void PhysicsDebugRender::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color){
        std::vector<Vector2f> arr(vertexCount);
        
        for(size_t i = 0; i < vertexCount; i++){
            arr[i] = { vertices[i].x, vertices[i].y };
        }

        batch.set_color({ color.r, color.g, color.b, color.a });
        batch.draw_polygon(arr);
    }

    /// Draw a circle.
    void PhysicsDebugRender::DrawCircle(const b2Vec2& center, float radius, const b2Color& color){
        batch.set_color({ color.r, color.g, color.b, color.a });
        // batch.draw_circle({ center.x, center.y }, radius, 0.f, 40);
    }

    /// Draw a solid circle.
    void PhysicsDebugRender::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color){
        batch.set_color({ color.r, color.g, color.b, color.a });
        // batch.draw_circle({ center.x, center.y }, radius, 0.f, 40);
    }

    /// Draw a line segment.
    void PhysicsDebugRender::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color){
        batch.set_color({ color.r, color.g, color.b, color.a });
        // batch.draw_line({ p1.x, p1.y }, { p2.x, p2.y });
    }

    /// Draw a transform. Choose your own length scale.
    /// @param xf a transform.
    void PhysicsDebugRender::DrawTransform(const b2Transform& xf){
        b2Vec2 p = xf.p;
        b2Vec2 px = p + (0.5f * xf.q.GetXAxis());
        b2Vec2 py = p + (0.5f * xf.q.GetYAxis());

        batch.set_color({ 1, 0, 0, 1 });
        // batch.draw_line({ p.x, p.y }, { px.x, px.y });
        // batch.draw_line({ p.x, p.y }, { py.x, py.y });
    }

    /// Draw a point.
    void PhysicsDebugRender::DrawPoint(const b2Vec2& p, float size, const b2Color& color){
        batch.set_render_type(ShapeBatch::RenderType::FILL);
        batch.set_color({ color.r, color.g, color.b, color.a });
        // batch.draw_circle({ p.x, p.y }, size * 0.01f, 0.f, 40);
        batch.set_render_type(ShapeBatch::RenderType::LINE);
    }
};