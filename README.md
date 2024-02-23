# Space game

## File structure
- Images/Audio/Files in the game: `assets/`
- Raw asset sources in the game: `resources/`
- Source code: `src/`

## Engine goals, loose things to do in free time
- [ ] Rendering api
- [ ] Animation api
- [ ] UI api
- [ ] Audio api
- [ ] Asset api
- [ ] Physics api
- [ ] Input api
- [ ] Component api

## Goals
- [ ] Create basic player and ship
- [ ] Create planets and gravity
- [ ] Create landable planets

## Theories
- Planets dont need a collider, allowing them to be massive.
    This is because to keep within the 0.1-10 meter guidelines
    of box2d, only physics objects should be simulated.
    Planets will be converted to individual edge shape
    colliders as the player 'enters' a planet scene.