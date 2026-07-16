# Draft Engine 2
The goal of Draft Engine 2.0 is to create a simple C++23 game engine with a clean architecture, tooling, and a basic editor.

### Core Principles
* Use modern C++23 features where they improve readability and correctness.
* Favor RAII and strong ownership semantics.
* Use exceptions for operations where failures are exceptional (filesystem, networking, serialization, etc.).
* Keep runtime systems (rendering, ECS, physics, math) exception-free during normal execution.
* Prefer explicit registration over implicit "magic."
* Editor and build tools work with the command line.
* The runtime should never know whether it is running inside the game or the editor.
* Make the engine one giant package that includes everything needed for a game.
* Intentionally lightweight engine, where main logic is all compiled in with C++.

## Current Todo/Future Ideas
- [ ] Fully implement RmlUI using engine architecture not glfw. Also add add_context to the RmlUISystem.
- [ ] Proper editor
- [ ] Shader pipeline
- [ ] Animation editor
- [ ] Particle editor
- [ ] Collider physics editor (overlaid texture)
- [ ] Hot-reload of the game module
- [ ] Prefab system
- [ ] Visual debugging tools
- [ ] Create a localization system
- [ ] Profiler system to standardize calls to RAM/CPU usage across systems
- [ ] Math utils for map, lerp, slerp, and other graphs
- [ ] Crash logs


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


## Engine Modules
```text
draft/
├── runtime/
├── editor/
├── build_tools/
├── common/
└── CMakeLists.txt
```

### Runtime
**Responsible for**:
* ECS
* Rendering
* Scene management
* Asset loading
* Serialization
* Reflection
* Physics
* Audio
* Input
* Ease-of-use UI layers
* RMLUI system
    - Should expose an add context

The Runtime contains no editor-specific functionality.

### Editor
A lightweight frontend for working with game projects.

**Responsible for**:
* Scene hierarchy
* Entity inspector
* Component editing
* Asset browser
* Scene view
* Transform gizmos
* Console output
* Build/export buttons

The editor operates directly on loose project files.  
The editor does **not** replace VS Code or CMake.

### Build Tools
Shared build/export functionality.
This will be a single CLI tool which the editor will make calls to, instead of compiling against the build_tools itself.
This CMake subproject should also contain the launcher executable. This is the program that in
- DEBUG: will load the game executable DLL/shared object and launch the game.
- RELEASE: will statically link the game library and launch the game.

Responsibilities:
* Pack project assets
    - Takes all loose assets (JSON/Binary), loads them to confirm valid, saves into temp packing folder (binary format)
    - After all assets validated and resaved, zip the whole assets folder into a .apak (asset pack)
    - Implement file provider for AssetFileSystem which takes in an assetpak filehandle
    - AssetFileSystem decompresses in memory, and then the assetpackfileprovider will supply the data as requested
    - Keep log in console for progress
* Export projects
    - Instructs CMake to build Release type and make the launcher statically link against the game
* Asset validation
    - Just makes sure it loads
* Dependency analysis (future)
* Asset importing/conversion (future)

The BuildTools is a CLI used by the user or editor.
There should only be one implementation of project export logic.


## Development Workflow
Normal development remains command-line driven.

```bash
cmake -S . -B build
cmake --build build
./build/game
```

The editor simply automates these tasks where convenient.

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
extern "C" void RegisterGame(Engine& engine)
{
    engine.registerComponent<Player>();
    engine.registerComponent<Health>();

    engine.registerSystem<PlayerMovement>();
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
The exporter packages them into a runtime format, which will probably be a simple custom zip for now.


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

If no compiled module exists, the editor should allow the user to build the project using the existing CMake configuration.

## Roadmap

### Phase 1, Engine 2.0
Goals:
* Modernize architecture
* Improve error handling
* Refactor filesystem
* Reflection system
* Serialization
* Scene system
* Component registry
* System registry
* Unit testing
* Documentation
* Build cleanup

Deliverable:  
A stable runtime suitable for future games.

### Phase 2, Editor
Goals:
* Open project directories
* Browse assets
* Edit scenes
* Entity hierarchy
* Component inspector
* Scene saving
* Build integration
* Export integration

Deliverable:  
A usable editor for content creation.

### Phase 3, Game Migration
Goals:
* Port the existing game to Engine 2.0
* Create scenes using the editor
* Continue gameplay development
* Expand tooling as needed

Deliverable:  
Gameplay development occurs primarily through the editor while code continues to be written in VS Code.