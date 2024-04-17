#pragma once

#include <memory>

namespace Draft {
    class Joint {
    public:
        // Constructors
        Joint();
        Joint(const Joint& other) = delete;
        ~Joint();

        // Functions
        
    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};