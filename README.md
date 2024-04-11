# Draft Engine

## Engine goals, loose things to do in free time
- [x] Logging system ingame
- [x] Rendering api
- [x] Asset api
- [x] Input api
- [ ] UI api
- [ ] Component api
- [ ] Physics api
- [ ] Scenegraph
- [ ] Audio api
- [ ] Animation api

## How the scene system works
- Each scene has its own registry
- Each scene has a render function and update function
- Entities are rendered/updated automatically
- Render/update function in scene is meant for custom rendering

## Instanced rendering
- An instanced renderer takes a set of base vertices and indices, aka a mesh  
and performs the model matrix on the CPU and buffers the data to reduce every
call to a singular call. This should be generalized enough to work for both
2D and 3D.

## Resources:
