#include "draft/math/bounds.hpp"

bool Draft::Math::contains(const Draft::Bounds& bounds, const Draft::Vector2f& point){
    // Runs a raycast bounds check for the point supplied
    size_t vertexCount = bounds.size();
    size_t intersections = 0;

    for(size_t i = 0; i < vertexCount; i++){
        // Wrapping traversal of a 1D array
        const auto& v0 = bounds[i];
        const auto& v1 = bounds[(i + 1) % vertexCount];

        // Skip if the vertices have no chance of intersecting
        if(point.y > v0.y && point.y > v1.y)
            continue;

        if(point.y < v0.y && point.y < v1.y)
            continue;

        if(point.x > v0.x && point.x > v1.x)
            continue;

        // Guaranteed intersection
        if(point.x <= v0.x && point.x <= v0.x){
            intersections++;
            continue;
        }

        // Run intersection 
        float slope = (v1.y - v0.y) / (v1.x - v0.x);
        float intersectionAtX = (point.y - v0.y) / slope + v0.x;

        // Check if its to the right
        if(intersectionAtX >= point.x)
            intersections++;
    }

    return (intersections % 2 != 0);
}