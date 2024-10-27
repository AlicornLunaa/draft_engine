#pragma once

namespace Draft {
    class Framebuffer {
    private:
        // Variables
        unsigned int fbo;
        unsigned int rbo;
        unsigned int tex;

    public:
        // Constructors
        Framebuffer();
        ~Framebuffer();

        // Functions
        void clear();
        void bind() const;
        void unbind() const;
    };
};