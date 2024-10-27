#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/asset_manager.hpp"
#include "draft/math/glm.hpp"
#include <memory>

namespace Draft {
    // Private functions
    void Batch::update_combined(){
        combinedMatrix = Matrix4(1.f);
        combinedMatrix *= projMatrix;
        combinedMatrix *= transMatrix;
    }

    // Constructors
    Batch::Batch(size_t maxSprites, std::shared_ptr<Shader> shaderPtr) : maxSprites(maxSprites), shaderPtr(shaderPtr ? shaderPtr : Assets::manager.get<Shader>("assets/shaders/default", true)) {
    }

    Batch::~Batch(){
    }

    // Functions
    void Batch::set_proj_matrix(const Matrix4& m){
        projMatrix = m;
        update_combined();
    }

    void Batch::set_trans_matrix(const Matrix4& m){
        transMatrix = m;
        update_combined();
    }
};