#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>


using namespace std;

namespace Draft {
    // Static data
    StaticResource<Shader> SpriteBatch::defaultShader = {FileHandle("assets/shaders/default")};

    // Constructor
    SpriteBatch::SpriteBatch() : Batch(), SpriteCollection() {
    }
    
    // Functions
    void SpriteBatch::flush(){
        SpriteCollection::flush();
    }

    void SpriteBatch::set_proj_matrix(const Matrix4& m){
        SpriteCollection::set_proj_matrix(m);
    }
    
    void SpriteBatch::set_trans_matrix(const Matrix4& m){
        SpriteCollection::set_trans_matrix(m);
    }
};