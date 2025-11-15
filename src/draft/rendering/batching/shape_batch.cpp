#include "draft/rendering/batching/shape_batch.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/batching/shape_collection.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/file_handle.hpp"
#include "glad/gl.h"
#include <tracy/Tracy.hpp>

using namespace std;

namespace Draft {
    // Static data
    StaticResource<Shader> ShapeBatch::defaultShader = {FileHandle("assets/shaders/shapes")};

    // Constructor
    ShapeBatch::ShapeBatch(Resource<Shader> shader) : Batch(), ShapeCollection(shader) {
    }

    // Functions
    void ShapeBatch::flush(){
        ShapeCollection::flush();
    }

    void ShapeBatch::set_proj_matrix(const Matrix4& m){
        ShapeCollection::set_proj_matrix(m);
    }
    
    void ShapeBatch::set_trans_matrix(const Matrix4& m){
        ShapeCollection::set_trans_matrix(m);
    }
};