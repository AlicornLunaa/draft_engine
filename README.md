# Draft Engine

## Engine goals, loose things to do in free time
- [x] Logging system ingame
- [ ] Rendering api
- [ ] Asset api
- [ ] Input api
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