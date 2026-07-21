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
- [ ] RMLUI editor with hot reload for RML documents and a data binding mockup
    * Implement the RMLUI editor view the engine editor. This should be a panel that is tabbed next to viewport, but invisible by default. It should be opened when double clicking an RML document. It will then open this RML document and show a preview on the screen for how it renders. There should be a way to define a data binding model using custom JSON so that can be previewed. It should update the preview on file save, such that the document changing itself or any document it links to changes should update the view immediately (look at game module watcher). Custom data models should be saved as a json in .draft-editor for future use when the document gets reopened
- [ ] Update comments. This project is almost ready for its version 2 release, but I would like all the comments updated. Get a checklist of every file in the engine and work file by file. The comments should be documenting enough for a wiki autogenerator to be used on the code. Do not reference phases or documentation that doesn't matter. Only talk about the code if it needs explanation. You can shorten comments if they are too long or verbose. Fix grammar issues and do not hyphenate. Speak like a human when writing comments. Make sure to use standard doxygen comments. Determine a consistent style guide to write into the information/style_guide.md based on the current code.

## Known bugs
- [ ] Particle editor doesn't show particle at (0, 0)
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