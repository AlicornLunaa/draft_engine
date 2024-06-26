# Draft Engine
Just a project to learn computer graphics, openGL, and C++ library creation.

## Engine goals, loose things to do in free time
- [x] Logging system ingame
- [x] Rendering api
- [x] Asset api
- [x] Input api
- [x] UI api
- [x] Component api
- [x] Physics api
- [x] Refactor rendering
- [x] Audio api
- [x] Particles
- [ ] Animation api
- [ ] Profiler api
- [ ] Scenegraph
- [ ] Localization API
- [ ] Handle errors gracefully

## How the engine works
### OpenGL Resources
- Every OpenGL class has had RAII in mind.
- Do not instantiate an OpenGL class without having an OpenGL context ready!

### Asset manager
- The asset manager works using whats called asset packages. This is an ID
which corresponds to current loaded assets. An asset package can be deleted which will unload only the assets in the package.
- The function called `start_package()->size-t` starts a new package
- The function called `select_package(size-t)` selects the package as current context
- The function called `end_package(size_t)` deselects current package and unloads the
assets it is currently using, if and only if no other resources are using it.

### File handle
- File handle controls reading and writing
- Access type LOCAL means relative to the executable
- Access type INTERNAL means compiled into the exe

### Inputs
- Keyboard and Mouse classes are static singletons
- Controls the interface with GLFW's input system
- Has support for callbacks or direct checking
- Before use, init should be called. Application does this automatically
- After use, cleanup should be called. Application does this automatically

### Rendering
- An instanced renderer takes a set of base vertices and indices, aka a mesh  
and performs the model matrix on the CPU and buffers the data to reduce every
call to a singular call. This should be generalized enough to work for both
2D and 3D.

### Application
- The fundamental class for the engine
- Handles updates, rendering, and events
- Funnels stuff to the active scene

### Scenes
- Each scene has its own registry
- Each scene has a render function and update function
- Entities are rendered/updated automatically
- Render/update function in scene is meant for custom rendering

### User interface
- Each panel has the offset for the buffer data and the length to subbuffer
- UI Container class holds the vertex buffers
- Each panel, when invalidated, rebuilds its buffer data and sends it to the GPU.
    Panels can define multiple vertices to draw
- Panels can hold a pointer to their parent, if the parent is invalidated, so is the child.
    Parent->child invalidations are checked in the ui container
    