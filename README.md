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
- [x] Animation api
- [ ] Profiler api
- [ ] Scenegraph
- [ ] Localization API
- [ ] Handle errors gracefully

## Oversights to fix
- [x] draw_centered_text for TextRenderer
- [x] font class needs to handle everything better, should be a single texture updated on demand
- [x] figure whatever is wrong with the coordinate system
- [x] updates need a timestep, not a deltatime
- [x] missing `on_attach` and `on_detach` methods for scenes
- [x] fix up rendering pipeline
- [x] maybe condense widgets and interface into the same directory?
- [x] asyncronous loading of assets, in case of OpenGL, loading raw data first
- [ ] just a better animation class
- [x] loading particles from files
- [ ] namespace inconsistencies for the UI
- [ ] more widgets for UIContainer class
- [ ] implement styling for UIContainer
- [ ] loading colliders from files
- [ ] better handling of references vs pointer

## How the engine works
### OpenGL Resources
- Every OpenGL class has had RAII in mind.
- Do not instantiate an OpenGL class without having an OpenGL context ready!

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
    
### Editor
- Scenes are collection of entities
    * Decoder class for abstract data to classes