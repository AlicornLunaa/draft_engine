#include "draft/rendering/batching/batch.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    // Private vars
    Batch* Batch::activeBatch = nullptr;

    // Private functions
    void Batch::update_combined(){
        combinedMatrix = Matrix4(1.f);
        combinedMatrix *= projMatrix;
        combinedMatrix *= transMatrix;
    }

    // Constructors
    Batch::Batch(Resource<Shader> shader) : shader(shader) {
    }

    // Functions
    void Batch::set_blending(bool blend){
        if(activeBatch == this)
            flush();
        
        this->blend = blend;
    }

    void Batch::set_shader(Resource<Shader> shader){
        if(activeBatch == this)
            flush();

        this->shader = shader;
        shader.get().bind();
    }

    void Batch::set_proj_matrix(const Matrix4& m){
        if(activeBatch == this)
            flush();

        projMatrix = m;
        update_combined();
    }

    void Batch::set_trans_matrix(const Matrix4& m){
        if(activeBatch == this)
            flush();

        transMatrix = m;
        update_combined();
    }

    void Batch::begin(){
        assert(activeBatch == nullptr && "Previous batch must be stopped before you can start a new one.");
        activeBatch = this;
    }

    void Batch::end(){
        assert(activeBatch != nullptr && "Batch must be started before it can be stopped.");
        assert(activeBatch == this && "Previous batch must be stopped before you can end this one.");
        flush();
        activeBatch = nullptr;
    }
};