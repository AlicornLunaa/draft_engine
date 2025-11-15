#include "draft/rendering/batching/collection.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    // Private functions
    void Collection::update_combined(){
        combinedMatrix = Matrix4(1.f);
        combinedMatrix *= projMatrix;
        combinedMatrix *= transMatrix;
        p_matricesDirty = true;
    }

    // Functions
    void Collection::set_proj_matrix(const Matrix4& m){
        projMatrix = m;
        update_combined();
    }

    void Collection::set_trans_matrix(const Matrix4& m){
        transMatrix = m;
        update_combined();
    }
};