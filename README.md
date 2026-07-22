# Draft Engine
Draft engine is a simple C++ based game engine. The main idea/architecture behind this project is that the game is the files. The engine is a framework the game calls into, and the editor is a simple interface for placing entities.

### Core Principles
* Use modern C++23 features where they improve readability and correctness.
* Favor RAII and strong ownership semantics.
* Use exceptions for operations where it makes sense (filesystem, networking, serialization, etc.).
* Make the engine one giant package that includes everything needed for a game.
* Intentionally lightweight engine, where main logic is all compiled in with C++.

### Known bugs
- [ ] Release build of editor doesn't work with debug build of game

## Project Structure
Example game project:

```text
MyGame/
├── src/
├── assets/
├── vendor/
│   └── draft/
├── CMakeLists.txt
└── build/
```

The engine is included as a git submodule inside `vendor/`.  
This should allow me to maintain a nice git history for my games.


## Engine Modules
```text
draft/
├── runtime/
├── editor/
├── build_tools/
├── common/
└── CMakeLists.txt
```

### Common
**Responsible for**:
* Serialization
* Reflection
* Math helpers
* File system
* Logging

### Runtime
**Responsible for**:
* ECS
* Rendering
* Scene management
* Asset loading
* Physics
* Audio
* Input
* Ease-of-use UI layers
* RMLUI system

The Runtime contains no editor-specific functionality.

### Editor
A lightweight frontend for working with game projects.

**Responsible for**:
* Scene hierarchy
* Entity inspector
* Component editing
* Assigning systems to a scene
* Asset browser
* Scene view
* Transform gizmos
* Console output
* Build/export buttons

The editor operates directly on loose project files.  
The editor does **not** replace VSCode, CMake, or art programs.

### Build Tools
Shared build/export functionality.  
This will be a single CLI tool which the editor will make calls to, instead of compiling against the build_tools itself.  
This CMake subproject should also contain the launcher executable. This is the program that in
- DEBUG: will load the game executable DLL/shared object and launch the game.
- RELEASE: will statically link the game library and launch the game.

**Responsible for**:
* Pack project assets
* Export projects
* Asset validation
* Dependency analysis (future)
* Asset importing/conversion (future)

The BuildTools is a CLI used by the user.

## Development Workflow
Normal development remains command-line driven.

```bash
cmake -S . -B build
cmake --build build
./build/game
```

The editor simply automates these tasks where convenient.  
This is also how an empty project is started, as the editor is built with the engine.

For example:
* Build
* Rebuild
* Export
* Run

All of these ultimately invoke the existing CMake workflow.


## Game Registration
The engine does not know about game-specific components or systems.  
The game explicitly registers everything during initialization.

Example:

```cpp
void register_game(GameContext& context, Scene& initialScene) {
    AssetManager& assets = context.assets;
    assets.queue<Texture>("assets/dev_texture.png");
    assets.load();

    Engine& engine = context.engine;
    engine.components().register_component<Player>();
    engine.components().register_component<Health>();
    engine.systems().register_system<PlayerMovement>([Scene&](){ ... });

    DRAFT_GAME_INFO("My Game", 1280, 720)
    DRAFT_GAME_MODULE(register_game)
}
```

Reflection supplies metadata used by:

* Serialization
* Scene loading
* Inspector
* Property editing


## Asset Philosophy
Everything is considered an asset.

Examples:

```text
assets/
├── scenes/
├── textures/
├── audio/
├── prefabs/
├── materials/
└── shaders/
```

Scenes are simply another asset type.  
The editor edits loose assets.  
The exporter packages them into a runtime format, which will be a simple custom zip for now.


## Export Pipeline
Development:
* Loose files
* JSON scenes
* Easy Git diffs
* Human-readable assets

Release:
* Assets packed into a custom binary/zip format
* Runtime loads packaged assets
* Faster loading
* Simpler distribution


## Editor Workflow
The editor opens a project root.  
Example:

```text
MyGame/
```

On startup the editor:
1. Opens the project.
2. Loads loose assets.
3. Detects the compiled game module.
4. Loads the game module.
5. Calls the game's registration function.
6. Discovers all registered components and systems.