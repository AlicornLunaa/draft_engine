# Draft Engine
Just a project to learn computer graphics, openGL, and C++ library creation.

## Engine goals, loose things to do in free time
- [x] Logging system ingame
- [x] Rendering api
- [x] Asset api
- [x] Input api
- [ ] UI api
- [x] Component api
- [x] Physics api
- [ ] Scenegraph
- [ ] Audio api
- [ ] Animation api
- [ ] Profiler API

## How the engine works
### OpenGL Resources
- Every OpenGL class has had RAII in mind.
- Do not instantiate an OpenGL class without having an OpenGL context ready!

### Asset manager
- Queue in different resources to load
- Loading is not done until load() is called

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
