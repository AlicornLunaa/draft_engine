#pragma once

#include "draft/rendering/mesh.hpp"
#include "draft/util/file_handle.hpp"

#include <vector>

namespace Draft {
    class Model {
    private:
        // Variables
        std::vector<DrawableMesh> meshes;

        // Private functions
        size_t ComponentTypeByteSize(int type);

    public:
        // Constructors
        Model();
        Model(const FileHandle& file);

        // Functions
        void render() const;
    };
};