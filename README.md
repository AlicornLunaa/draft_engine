# Draft Engine v0.1.0
Just a project to learn computer graphics, openGL, and C++ library creation.

## Next version goals
- [ ] Profiler api
- [x] Better asyncronous loading of resources
- [ ] Texture packer
- [ ] Develop animation class
- [ ] Localization API
- [ ] Handle errors gracefully
- [x] Overhaul UI
- [ ] Load collider data from files
- [x] Serialization
- [ ] JSON handling
- [ ] Update to new physics engine
- [x] Better texture class with some sort of TextureProperties struct
- [ ] Same with framebuffer
- [ ] Crash logs
- [ ] Devtools & registry editor
- [ ] Math utils for lerp, slerp, and other graphs. Map function as well
- [ ] Color class

## How the engine works
### OpenGL Resources
- Every OpenGL class has had RAII in mind.
- Do not instantiate an OpenGL class without having an OpenGL context ready!

### File handle
- File handle controls reading and writing
- Access type LOCAL means relative to the executable
- Access type INTERNAL means compiled into the exe

### Inputs
- Keyboard and Mouse classes are classes which help abstract GLFW's functoins
- Controls the interface with GLFW's input system
- Has support for callbacks or direct checking

### Rendering
- Done either using the shapebatch, spritebatch, or vertexarray.
- ShapeBatch and SpriteBatch are extremely generalized, and therefore slow
- VertexArray must be extremely specific, therefore faster

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
- Each panel, when invalidated, rebuilds its buffer data and sends it to the GPU. Panels can define multiple vertices to draw
- Panels can hold a pointer to their parent, if the parent is invalidated, so is the child. Parent->child invalidations are checked in the ui container
    
### Editor
- Scenes are collection of entities
    * Decoder class for abstract data to classes
