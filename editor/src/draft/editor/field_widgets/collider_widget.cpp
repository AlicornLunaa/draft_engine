#include "draft/editor/field_widgets/collider_widget.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/physics/shapes/chain_shape.hpp"
#include "draft/physics/shapes/circle_shape.hpp"
#include "draft/physics/shapes/edge_shape.hpp"
#include "draft/physics/shapes/polygon_shape.hpp"

#include "imgui.h"

namespace Draft {
    namespace {
        const char* shape_type_name(ShapeType type){
            switch(type){
                case ShapeType::CIRCLE: return "Circle";
                case ShapeType::POLYGON: return "Polygon";
                case ShapeType::EDGE: return "Edge";
                case ShapeType::CHAIN: return "Chain";
            }

            return "Unknown";
        }

        bool draw_shared_shape_fields(Shape* shape){
            bool changed = false;

            float friction = shape->friction;
            if(ImGui::DragFloat("Friction", &friction, 0.01f, 0.f, 10.f)){ shape->friction = friction; changed = true; }

            float restitution = shape->restitution;
            if(ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.f, 10.f)){ shape->restitution = restitution; changed = true; }

            float density = shape->density;
            if(ImGui::DragFloat("Density", &density, 0.01f, 0.f, 1000.f)){ shape->density = density; changed = true; }

            bool isSensor = shape->isSensor;
            if(ImGui::Checkbox("Is Sensor", &isSensor)){ shape->isSensor = isSensor; changed = true; }

            return changed;
        }

        bool draw_circle_fields(CircleShape* circle){
            bool changed = false;

            Vector2f position = circle->get_position();
            if(ImGui::DragFloat2("Position", &position.x, 0.1f)){ circle->set_position(position); changed = true; }

            float radius = circle->get_radius();
            if(ImGui::DragFloat("Radius", &radius, 0.05f, 0.01f, 1000.f)){ circle->set_radius(radius); changed = true; }

            return changed;
        }

        bool draw_polygon_fields(PolygonShape* polygon){
            bool changed = false;
            int removeIndex = -1;

            for(size_t i = 0; i < polygon->get_vertex_count(); i++){
                ImGui::PushID(static_cast<int>(i));

                Vector2f vertex = polygon->get_vertex(i);
                std::string label = "Vertex " + std::to_string(i);

                if(ImGui::DragFloat2(label.c_str(), &vertex.x, 0.1f)){ polygon->set_vertex(i, vertex); changed = true; }

                ImGui::SameLine();
                if(ImGui::SmallButton("Remove")) removeIndex = static_cast<int>(i);

                ImGui::PopID();
            }

            if(removeIndex >= 0){ polygon->del_vertex(removeIndex); changed = true; }
            if(ImGui::SmallButton("Add Vertex")){ polygon->add_vertex({0.f, 0.f}); changed = true; }

            return changed;
        }

        bool draw_edge_fields(EdgeShape* edge){
            bool changed = false;

            Vector2f start = edge->get_start();
            if(ImGui::DragFloat2("Start", &start.x, 0.1f)){ edge->set_start(start); changed = true; }

            Vector2f end = edge->get_end();
            if(ImGui::DragFloat2("End", &end.x, 0.1f)){ edge->set_end(end); changed = true; }

            return changed;
        }

        bool draw_chain_fields(ChainShape* chain){
            // ChainShape has no in-place point setter, only add()/remove()/clear(), so an edited
            // point is applied by rebuilding the whole list with that one entry replaced.
            bool changed = false;
            std::vector<Vector2f> points = chain->get_points();
            int removeIndex = -1;

            for(size_t i = 0; i < points.size(); i++){
                ImGui::PushID(static_cast<int>(i));

                Vector2f point = points[i];
                std::string label = "Point " + std::to_string(i);

                if(ImGui::DragFloat2(label.c_str(), &point.x, 0.1f)){ points[i] = point; changed = true; }

                ImGui::SameLine();
                if(ImGui::SmallButton("Remove")) removeIndex = static_cast<int>(i);

                ImGui::PopID();
            }

            if(removeIndex >= 0){ points.erase(points.begin() + removeIndex); changed = true; }
            if(ImGui::SmallButton("Add Point")){ points.push_back({0.f, 0.f}); changed = true; }

            if(changed){
                chain->clear();
                for(const Vector2f& point : points) chain->add(point);
            }

            return changed;
        }

        bool draw_shape_fields(Shape* shape){
            bool changed = draw_shared_shape_fields(shape);

            switch(shape->type){
                case ShapeType::CIRCLE: changed |= draw_circle_fields(static_cast<CircleShape*>(shape)); break;
                case ShapeType::POLYGON: changed |= draw_polygon_fields(static_cast<PolygonShape*>(shape)); break;
                case ShapeType::EDGE: changed |= draw_edge_fields(static_cast<EdgeShape*>(shape)); break;
                case ShapeType::CHAIN: changed |= draw_chain_fields(static_cast<ChainShape*>(shape)); break;
            }

            return changed;
        }
    }

    bool draw_component_widget(FieldContext& ctx, std::string_view label, Collider& collider){
        bool changed = false;
        ImGui::PushID(label.data(), label.data() + label.size());

        Vector2f position = collider.get_position();
        if(ImGui::DragFloat2("Position", &position.x, 0.1f)){ collider.set_position(position); changed = true; }

        Vector2f origin = collider.get_origin();
        if(ImGui::DragFloat2("Origin", &origin.x, 0.1f)){ collider.set_origin(origin); changed = true; }

        Vector2f scale = collider.get_scale();
        if(ImGui::DragFloat2("Scale", &scale.x, 0.05f)){ collider.set_scale(scale); changed = true; }

        float rotation = collider.get_rotation();
        if(ImGui::DragFloat("Rotation", &rotation, 0.01f)){ collider.set_rotation(rotation); changed = true; }

        ImGui::Separator();

        ColliderShapeSelection& shapeSelection = ctx.app.colliderShapeSelection;
        ImGui::Checkbox("Edit Shapes", &shapeSelection.editing);

        if(!shapeSelection.editing){
            ImGui::PopID();
            return changed;
        }

        ImGui::Indent();

        int shapeCount = static_cast<int>(collider.get_shape_count());
        if(shapeSelection.shapeIndex >= shapeCount) shapeSelection.shapeIndex = -1;

        Shape* toRemove = nullptr;

        for(int i = 0; i < shapeCount; i++){
            Shape* shape = collider.get_shapes()[i].get();
            std::string entryLabel = std::string(shape_type_name(shape->type)) + " " + std::to_string(i);

            ImGui::PushID(i);
            if(ImGui::Selectable(entryLabel.c_str(), shapeSelection.shapeIndex == i))
                shapeSelection.shapeIndex = i;

            ImGui::SameLine();
            if(ImGui::SmallButton("Remove")) toRemove = shape;
            ImGui::PopID();
        }

        if(toRemove){
            collider.del_shape(toRemove);
            shapeSelection.shapeIndex = -1;
            changed = true;
        }

        if(ImGui::SmallButton("Add Shape"))
            ImGui::OpenPopup("AddShapePopup");

        if(ImGui::BeginPopup("AddShapePopup")){
            if(ImGui::MenuItem("Circle")){
                CircleShape shape;
                collider.add_shape(shape);
                changed = true;
            }

            if(ImGui::MenuItem("Box")){
                PolygonShape shape;
                shape.set_as_box(0.5f, 0.5f);
                collider.add_shape(shape);
                changed = true;
            }

            if(ImGui::MenuItem("Edge")){
                EdgeShape shape({-0.5f, 0.f}, {0.5f, 0.f});
                collider.add_shape(shape);
                changed = true;
            }

            if(ImGui::MenuItem("Chain (Loop)")){
                ChainShape shape(ChainShape::LOOP);
                collider.add_shape(shape);
                changed = true;
            }

            ImGui::EndPopup();
        }

        if(shapeSelection.shapeIndex >= 0 && shapeSelection.shapeIndex < static_cast<int>(collider.get_shape_count())){
            ImGui::Separator();
            ImGui::PushID("SelectedShape");
            bool shapeChanged = draw_shape_fields(collider.get_shapes()[shapeSelection.shapeIndex].get());
            ImGui::PopID();

            // Editing a shape's own fields (radius, vertices, ...) mutates it directly, unlike
            // Collider's own set_position()/add_shape()/del_shape(), which already reattach
            // themselves. Nothing else rebuilds the live fixtures from the new shape data, so
            // that's on us here.
            if(shapeChanged){
                if(collider.is_attached())
                    collider.update_collider();

                changed = true;
            }
        }

        ImGui::Unindent();
        ImGui::PopID();
        return changed;
    }
}
