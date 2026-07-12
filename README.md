# Engine 2.0 (in progress)

This is the Engine 2.0 skeleton described in [`refactor/architecture.md`](../refactor/architecture.md),
built up side-by-side with the pre-refactor `engine/` so both can be referenced while code
migrates over. See [`refactor/checklist.md`](../refactor/checklist.md) for what's landed so far
and what's still pending.

Not wired into the repo's root `CMakeLists.txt` yet - build it standalone:

```bash
cd engine-refactor
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Modules
- `common/` - dependency-light utilities shared by Runtime and BuildTools. No knowledge of
  rendering, ECS, or anything game-specific.
- `runtime/` - rendering, ECS, scenes, ... move here from `engine/`. So far just asset loading
  (`AssetManager`, see "How Runtime works" below) - everything else is still a placeholder.
- `build_tools/` - placeholder for now (asset packing/export/validation, shared by CLI tools
  and the editor).

Editor isn't scoped yet; architecture.md defers it to Phase 2, after Runtime is stable.

## How Common works

### File stack
Old `FileHandle` tagged itself INTERNAL/LOCAL and branched on that tag internally. The
replacement is dependency-injection based:
- `FileProvider` is the abstract backing store (exists/read/write/list/...). New backends
  implement this interface rather than extending `FileHandle`.
- `DiskFileProvider` and `EmbeddedFileProvider` (cmrc-backed) are the two current
  implementations.
- `FileHandle` owns a *clone* of whichever provider it's given (see `FileProvider::clone()`),
  so it has ordinary value semantics and can be copied/stored freely without worrying about the
  provider's lifetime. It has no default constructor and no implicit single-argument
  constructor - constructing one with an empty path throws `std::invalid_argument` rather than
  producing a handle that silently does nothing.
- `AssetFileSystem` searches an ordered list of providers (disk, then embedded, by default) and
  resolves to whichever one actually has the file - a loose file on disk shadows the version
  baked into the binary. `open()` throws if the path isn't found in any registered provider.
- `HostFileSystem` only ever talks to disk, and is the one with write-side operations:
  open/read/write/copy/exists/create_directories/remove.
- `FileHandle::local()`/`::internal()`/`::automatic()` have been removed - use
  `HostFileSystem`, `AssetFileSystem`, or a provider's own `.open()` instead.

### Time
Reimplemented on `std::chrono::microseconds` instead of a pImpl wrapping SFML's `sf::Time` -
Common has no reason to depend on a windowing/audio library just to measure a duration. Same
public API (including the slightly unusual `void operator=` and non-`const` historical
signatures) as before, just a different backing store.

### JSON / Serializer
- `JSON` is a thin `nlohmann::json` subclass with an extra constructor that parses directly
  from a `FileHandle`.
- `JSONInterface` is a virtual encode()/decode() interface for types that need to be
  serialized through a common base pointer/reference (a heterogeneous collection of assets,
  say) - `encode_json()`/`decode_json()` are one-line forwarders onto it.
- `Serializer` (in `serializer.hpp`) is a separate, concept-driven mechanism for concrete,
  compile-time-known types: a type opts in by providing its own static
  `serialize(const T&, ...)`/`deserialize(T&, ...)` functions for `Binary::ByteArray` and/or
  `JSON`, and satisfies `BinarySerializable`/`JsonSerializable`/`Serializable` accordingly.
  Built-in overloads already handle trivially-copyable types and `std::vector<K>`.
  `deserialize_and_advance()` assumes a `BinarySerializable` type's binary encoding is exactly
  `sizeof(T)` bytes - a type with a variable-length encoding (e.g. anything holding a string)
  must advance its own span manually instead of going through that helper.

### Reflectable
A macro-based, name-capturing reflection system (`reflectable.hpp`), superseding the old
`DRAFT_ENABLE_REFLECTION(...)` placeholder (which only captured member pointers, not names).
There's no compiler-level reflection in C++23 to pull member names from automatically, so names
still have to be spelled out somewhere - the design puts that in one place per class:

```cpp
struct Point {
    DRAFT_REFLECTED(int, x) = 0;
    DRAFT_REFLECTED(int, y) = 0;

    DRAFT_REFLECTABLE(Point, x, y)
};
```

- `DRAFT_REFLECTED(Type, Name)` expands to a plain `Type Name` member declaration. It carries no
  metadata itself - it's an optional, purely visual marker that a member is part of the type's
  reflected state, matching up with an entry in that type's `DRAFT_REFLECTABLE(...)` list.
- `DRAFT_REFLECTABLE(ClassName, ...)` declares `static constexpr auto reflect()`, returning a
  `std::tuple<Field<ClassName, T1>, Field<ClassName, T2>, ...>` - one `Field` per listed member,
  each pairing that member's stringized name (`#member`) with a pointer-to-member
  (`&ClassName::member`). Internally it expands the variadic member list through a small
  recursive `__VA_OPT__`-based FOR_EACH (`DRAFT_DETAIL_FOR_EACH` and friends) rather than
  depending on an external macro library - it supports up to 256 fields (4 levels of quadruple
  expansion), tested past its first expansion boundary at 7 fields.
- `Field<Class, MemberType>` holds the `name` plus `get()`/`set()` accessors bound to a specific
  instance.
- `Draft::Reflectable<T>` is a concept satisfied by any type with a `reflect()` (i.e., anything
  that used `DRAFT_REFLECTABLE`).
- `Draft::for_each_field(instance, visitor)` calls `visitor(name, valueRef)` for every field, in
  declaration order - the generic-visitor case the old macro already supported, now with a name
  alongside each value. `Draft::visit_field(instance, name, visitor)` is the complementary
  by-name lookup (`visitor(valueRef)` for the one field matching `name`, returns whether it found
  one) - the piece serialization keys / an inspector / property editing actually need, which the
  old placeholder explicitly couldn't do.

### Logger
`Logger` (in `logger.hpp`) is a namespace of free functions - `print()`/`println()`/`print_raw()` -
unchanged in shape from the old engine. What changed:
- The level enum moved from a bare `Draft::Level` to `Draft::LogLevel`, since `Level` alone was
  too generic a name to sit at the top of the `Draft` namespace.
- The old implementation's `#else` branch (a plain, uncolored fallback gated behind an
  `ANSI_SUPPORT` macro that was unconditionally `#define`d right above it, so the branch could
  never actually compile) is gone; the ANSI-colored path was the only one that ever ran, so it's
  now just the only path.
- The full `ansi_colors.hpp` color table didn't come along - `logger.cpp` only needs five ANSI
  codes, kept as file-local constants, rather than pulling in a whole palette Common otherwise
  has no use for yet.
- Behavior is otherwise identical: every message goes to stdout, colored by level; `Warning`,
  `Severe`, and `Critical` also duplicate the message (uncolored) to stderr so it surfaces even
  if stdout is redirected elsewhere.

### Math
`glm.hpp` re-exports glm under the `Draft`/`Math` aliases (`Vector2f`, `Matrix4`, `Math::` for
`glm::`, ...), same shape as the old engine's. `funcs.hpp`, `bounds.hpp` (`Bounds` +
`glm::contains()`, a wrapping ray-cast point-in-polygon test), and `rect.hpp` (`Rect<T>` +
`glm::normalize_coordinates()`) all came along mostly unchanged. What's different:
- `Quaternion`/`Quaterniond` used to alias `Matrix<float/double, 4, 4>` - a 4x4 matrix, not a
  quaternion at all. Nothing in `engine/` actually used either alias (grepped for `Quaternion`,
  zero hits outside the declaration), so this was corrected to `Math::qua<float>`/`Math::qua<double>`
  (i.e. `glm::quat`/`glm::dquat`) rather than porting forward a mislabeled, unused type.
- Two pieces were deliberately *not* ported, because Common can't take on the dependency they'd
  drag in (per `architecture.md`, Common has "no knowledge of rendering, ECS, or anything
  game-specific"):
  - `vector2_p.hpp`/`vector3_p.hpp` (the `vector_to_b2()`/`b2_to_vector()` box2d conversion
    helpers) - only ever used from `engine/src/draft/phys`, so they belong with Physics in
    Runtime, not Common.
  - `rect.hpp`'s `normalize_coordinates(const RenderWindow&, ...)` overload - only the
    `Rect`-based overload it delegated to came along; the `RenderWindow` one belongs with
    Rendering in Runtime once that type exists there, and can just call Common's `Rect` overload
    the same way the old one did.
  Both are tracked as follow-ups in `refactor/checklist.md`.

### CircularBuffer, Clock, Color
Three more dependency-light `engine/util` pieces that fit Common, ported after a sweep of the
rest of `engine/` turned up nothing else that qualifies (everything else touches ECS, rendering,
box2d, or one of SFML/GLFW/OpenGL directly):
- `CircularBuffer<T>` (`circular_buffer.hpp`) - unchanged, a fixed-capacity ring buffer.
- `Clock` (`clock.hpp`/`.cpp`) - used to be a pImpl wrapping SFML's `sf::Clock`, the same pattern
  `Time` had before it moved to `std::chrono`. Now that `Time` no longer needs SFML, `Clock`
  doesn't either: it's a `std::chrono::steady_clock::time_point` directly, no pImpl.
- `Color` (moved to `math/color.hpp`/`.cpp`, since it's purely a `Vector4f` wrapper with hex
  string conversions - it belongs next to the other math types rather than in `util/`). Two
  changes from the old `engine/`:
  - It's now `Draft::Color` rather than a bare global `::Color` - every other utility in the
    engine lives in the `Draft` namespace; `Color` was the one exception, and left in the global
    namespace it's a generic enough name to invite collisions.
  - The `#RGBA` shorthand parse (e.g. `"#f00f"`) had a real bug: each channel's hex digit gets
    duplicated into a full byte directly (`"f"` -> `"ff"` -> 255), but the code then *also*
    multiplied that already-correct byte by 17 (the correction the neighboring `#RGB` branch
    needs, since *that* one parses a bare nibble). The result was every channel coming out 17x
    too large. Nothing caught this because the old test suite only ever exercised `#RRGGBB` and
    `#RRGGBBAA`, never the shorthand forms - a new `Color.FromShorthandHex` test exercises `#RGB`
    and `#RGBA` and would have caught it immediately.
  - Malformed hex (wrong length) used to hit an `assert` that's compiled out under `NDEBUG`,
    falling through to read `r`/`g`/`b` uninitialized - undefined behavior in release builds.
    It now throws `std::invalid_argument` unconditionally instead.

## How Runtime works

### Asset loading
`AssetManager` (`draft/asset/asset_manager.hpp`) replaces the old engine's `Assets` singleton -
a from-scratch redesign, not a port, after a review of the old design turned up a working but
fragile system (see `refactor/checklist.md`'s "Known issues" for the specific bugs that drove
each decision below).

- **Not a singleton.** `AssetManager` is an ordinary object - construct as many as you need
  (e.g. one per open project), each with its own `AssetFileSystem`.
- **Built on `AssetFileSystem`**, not the deprecated `FileHandle::automatic()`/`::local()`.
  Every load resolves through `AssetFileSystem::open()`, so a loose file on disk still shadows
  an embedded one, same as everywhere else in Common.
- **Exceptions are the error-reporting mechanism**, not an incidental side effect of the wrong
  container API. `get<T>()` throws (whatever the loader throws, or `std::logic_error` if no
  loader is registered for `T`) unless a placeholder is registered for `T`, in which case it
  logs and falls back instead. The batch APIs (`queue()`+`load()`/`load_async()`) deliberately
  don't propagate a single asset's failure - they log it, fall back to a placeholder or leave
  that key unloaded, and record it in `get_load_errors()`, so one bad asset can't abort an
  entire loading screen.
- **Loader registration can't reproduce the old design's `SoundBufferLoader` bug** (it registered
  itself under `typeid(SoundBufferLoader)` instead of `typeid(SoundBuffer)`, silently orphaning
  every sound loaded through the batch path). There's no virtual `BaseLoader` subclass hierarchy
  hand-hardcoding a `typeid()` anymore - `register_loader<T>()`/`register_async_loader<T>()`
  always derive the type key from the template parameter at the call site, so that whole class
  of typo is no longer expressible.
  - `register_loader<T>(LoadFn<T>)` registers a single synchronous function
    (`T(const FileHandle&, AssetManager&)` - the `AssetManager&` is there so a loader can
    resolve a referenced sub-asset, e.g. a particle system loading its texture, the same pattern
    the old `ParticleLoader` used). This alone is enough for `get<T>()` and `load()`.
  - `register_async_loader<T>(LoadFn<T>, OffThreadLoadFn<T>, FinishLoadFn<T>)` additionally
    registers a real two-stage async path: the off-thread stage runs on a worker thread and
    produces a type-erased (`std::any`) intermediate value, and must *not* touch the manager;
    the finish stage runs back on the owning thread and may call back into it freely, mirroring
    the old CPU-decode/GPU-upload split (e.g. `Image` on a worker thread, `Texture` on the main
    thread) without requiring every loader to hand-duplicate its parsing logic between a sync
    and an async copy the way most of the old loaders did.
  - If `load_async()` is asked to load a `T` with no async loader registered, it runs that one
    synchronously, inline, with a logged warning, rather than risking a plain `LoadFn` (which
    may call back into the manager) running unsynchronized on a worker thread.
- **`Resource<T>` is the "pointer-ish", swap-in-place handle** the old design's raw
  `AssetPtr*`-into-a-map trick was reaching for, minus the dangling-on-cleanup risk: it holds a
  `shared_ptr<AssetSlot<T>>`, and `AssetSlot<T>` wraps a `std::atomic<shared_ptr<T>>`.
  `reload<T>()`/`unload<T>()` (and a failed load falling back to a placeholder) all just call
  `set()` on the *same* slot, so every `Resource<T>` already handed out for that key observes
  the change without the caller ever re-requesting it from the manager - and because the slot
  is its own independently-refcounted heap object rather than a pointer into the manager's map,
  a `Resource<T>` outliving a `cleanup()`/`unload()` call sees a clean null instead of dangling.
  The atomic also makes reading a `Resource<T>` safe from any thread while the manager's owning
  thread reloads it - see the caveat on raw `T*`/`T&` lifetime in `resource.hpp`.
- **Threading model**: one `AssetManager` owns a small pool of `std::jthread` workers
  (`detail::JobRunner`, pImpl'd out of `asset_manager.hpp` into `asset_manager.cpp` so the
  public header doesn't drag in `<thread>`/`<mutex>`/`<condition_variable>`). Compared to the
  old design's one-`std::thread`-per-`load_async()`-call, detached and never joined: `jthread`
  auto-joins on destruction, and destruction order is arranged (the `JobRunner` member is
  declared last, so it's destroyed - and its workers joined - before anything they might
  reference) so an `AssetManager` going out of scope can never leave a worker thread touching
  freed memory. Worker threads only ever touch a job's own captured-by-value state plus
  `AssetFileSystem` (const, safe for concurrent reads); every mutation of shared registry state
  happens in a job's "finish" half, which only ever runs on the owning thread via `poll_async()`.
- **What's still missing**: per-asset streaming/priority isn't implemented (this covers what the
  old design's `unload()` TODO comment described - swap-to-placeholder-then-release - properly,
  but nothing beyond that); and the nine concrete loaders (Image, Texture, Font, ...) from the
  old engine haven't been ported yet, since their resource types (Rendering, Audio) don't exist
  in Runtime yet. Tests use synthetic types (`TextAsset`, `SlowAsset`, ...) to exercise the
  framework itself in the meantime.

### Rendering
`draft/rendering/` has `Window`, `RenderWindow`, and `Image`, ported from the old engine largely
unchanged - unlike ECS/AssetManager, none of these needed a redesign, just the dependencies wired
in and a couple of dead ends dropped.

- **`Window`** (`window.hpp`/`.cpp`) - a GLFW window: creation with configurable hints
  (`GLFWProperties`), resize/frame-resize/focus/close callbacks, title/position/size/vsync/
  fullscreen/open getters-setters, and static ref-counted GLFW init/terminate so multiple windows
  can coexist. `Keyboard*`/`Mouse*` stay as forward-declared raw pointers (nulled in the
  destructor) exactly like `FixtureDef::shape`/`GearJointData::joint1` earlier - neither
  `Keyboard` nor `Mouse` is ported yet, but nothing here needs their definition. `set_icon(const
  Image&)` is implemented for real now that `Image` exists (see below) - converts a copy to RGBA
  if needed and calls `glfwSetWindowIcon`. The old version was an unfinished stub (`exit(500)`,
  never actually implemented); `reset_icon()` never needed `Image` and was already in place.
- **`RenderWindow`** (`render_window.hpp`/`.cpp`) - a `Window` with an OpenGL context: loads GLAD
  (`gladLoadGL`), sets baseline GL state (depth test), and adds `clear()`/`display()`. Two changes
  from the old engine: GLAD failure now throws `std::runtime_error` instead of calling `exit(0)`
  from inside a constructor (matching `Window`'s own constructor and this project's
  exception-first style elsewhere - a bug fix, not a redesign), and the header no longer
  `#include`s `imgui_engine.hpp`/`rml_engine.hpp` - both were unused dead includes in the old
  version.
- **`Image`** (`image.hpp`/`.cpp`) - an in-memory pixel buffer (RAM only, no OpenGL involvement,
  so it can be built before a GL context exists) with full value semantics: construct from a
  solid color, raw bytes, compressed bytes (PNG/JPEG via stb_image), or a `FileHandle`; `save()`
  (PNG, via stb_image_write); `mask()`/`copy()`/`flip_horizontally()`/`flip_vertically()`/
  `set_pixel()`/`get_pixel()`/`convert()`. Needs `ColorFormat` (`aliasing/format.hpp`) and
  `IntRect`/`FileHandle` (already in Common). Three real bugs found and fixed while porting, all
  confirmed by dedicated tests failing before the fix:
  - `byte_to_float`/`float_to_byte` reinterpreted a byte through a `char&` - signed on this
    platform - so any channel value >= 128 (0x80) came back negative (`0xFF` read as `-1`, not
    `255`). Every pixel-touching test failed until this was `unsigned char`. This is the same
    category of bug as `Color`'s old `#RGBA`-shorthand parsing issue - a signedness slip, not a
    design question.
  - `Image::copy()`'s alpha-scalar lookup indexed `src.dataPtr[x + y * src.size.x + 3]`, missing
    the `* srcStride` every other index in that function uses - so `copy(..., applyAlpha=true)`
    read the wrong byte entirely for any source wider than one pixel. Fixed to match the
    `srcIndex` pattern used two lines above it; a dedicated regression test (a 2x2 source, a
    different alpha per pixel) fails under the old indexing and passes under the fix.
  - `Image::mask()` accepted an `alpha` parameter (defaulted to `std::byte{0x00}`) but its body
    always hardcoded `std::byte{0x0}` for a masked-out pixel's alpha, ignoring whatever the caller
    passed in. Fixed to actually use `alpha`.
  - stb_image/stb_image_write's implementation is compiled in its own dedicated
    `stb_image_impl.cpp`, not folded implicitly into some unrelated file - the old engine defines
    `STB_IMAGE_IMPLEMENTATION` in `texture.cpp` and `STB_IMAGE_WRITE_IMPLEMENTATION` in
    `model.cpp`, so `Image` there only links because a completely unrelated file happens to
    compile the implementation somewhere else in the binary.
- **Dependencies**: GLFW is fetched (`FetchContent`, pinned to `3.4`, same pattern as
  `entt`/`glm`/`nlohmann_json`); GLAD is vendored (`runtime/vendor/glad/`, copied unchanged from
  the old engine - it's a small pre-generated loader with no generator step, just committed
  `gl.c`/`gl.h` for `gl:core=4.6`); stb_image/stb_image_write are vendored too
  (`runtime/vendor/stb/`, just those two headers - not the old engine's entire ~20-header stb
  collection, which is mostly for Audio/Font, out of scope here). All three are linked `PRIVATE`
  to `draft_runtime` (the old engine linked GLFW/GLAD `PUBLIC`) - none of `window.hpp`/
  `render_window.hpp`/`image.hpp` expose a GLFW, GL, or stb type publicly, so there's no reason to
  leak any of them to consumers of `draft::runtime`.
- **Verified for real, not just compiled**: `Window`/`RenderWindow`/`set_icon` are the pieces with
  genuine runtime behavior tied to an external environment (a real GLFW window, a real GL driver),
  so a compile-only smoke check wouldn't have proven much. Confirmed with a real hidden window
  (`GLFW_VISIBLE` off) actually getting a live GL 4.6 core context from the system's real driver,
  `glGetString(GL_VERSION)` read back successfully; separately, confirmed `set_icon` genuinely
  calling through to `glfwSetWindowIcon` and succeeding under a real X11 backend (this system's
  default Wayland backend logs - correctly - that Wayland's protocol doesn't allow clients to set
  their own window icon at all; that's a real platform limitation, not a bug here). Both were
  throwaway verification code, deleted after confirming. The committed tests
  (`Window::get_default_properties()`, all of `Image`) are otherwise ordinary unit tests -
  `Image` has no external-environment dependency, so unlike `Window`/`RenderWindow` it gets full
  dedicated coverage; creating a real window/GL context still isn't something to unit-test in CI,
  matching the old engine's own precedent of zero tests for either.
- **`aliasing/*.hpp`**: the rest of the GL enum wrappers - `Filter`, `Wrap`,
  `FramebufferAttachment`, `FramebufferTarget`, `Parameter`, `TextureTarget` - are ported
  alongside `ColorFormat` now too. Same shape as `ColorFormat`/`PhysMask`/`Event`/`Action`: plain
  data (`GL_*` values aliased into `Draft` names), zero logic, no dedicated tests (nothing to
  verify beyond "the values match", confirmed via a `static_assert` smoke check against the real
  GLAD constants). `Texture` (below) is their first real consumer; `FrameBuffer`/`VertexBuffer`
  still wait.
- **`Texture`/`TextureRegion`** (`texture.hpp`/`.cpp`) - a live GL texture object:
  `TextureProperties` (target/format/size/parameters/data type) plus construction from defaults,
  an `Image`, or a `FileHandle`, `bind()`/`is_bound()`/`unbind()`, `set_image()` (sub-region
  upload), `set_properties()`, `reload()`. Three changes from the old engine:
  - `TextureRegion::get_uv()` returns `std::pair<Vector2f, Vector2f>` instead of
    `std::pair<ImVec2, ImVec2>` - the old file only `#include`d `imgui.h` for that one return
    type; nothing else here touches ImGui, which isn't wired into engine-refactor anyway (it's an
    Editor/interface-layer concern).
  - Adapted to this project's own `Resource<T>` (`draft/asset/resource.hpp`, from the
    `AssetManager` redesign): `get()` returns `T*` here, not the old `Resource<T>`'s `T&`, so
    `texture.get().get_properties()` became `texture->get_properties()`.
  - The default `TextureProperties::parameters` map listed `{TEXTURE_WRAP_S, REPEAT}` twice
    instead of once for S and once for T. `std::map`'s initializer-list constructor keeps the
    first occurrence of a duplicate key, so `TEXTURE_WRAP_T` was never actually in the map -
    though this turns out to have no observable effect today, since OpenGL's own default for
    `TEXTURE_WRAP_T` is already `GL_REPEAT`. Fixed anyway (now sets `TEXTURE_WRAP_T` explicitly,
    with a test pinning down the intended state) since the map should say what it means, and
    there's no guarantee a future default stays lined up with GL's by coincidence.
  - **Found and fixed a real, order-dependent bug** while writing `Texture`'s own tests (the old
    engine has none): the static `boundTextures` cache is keyed only by raw GL texture ID, and
    `cleanup()` never cleared a deleted texture's entry out of it. Since GL frequently reuses
    small integer IDs after `glDeleteTextures`, a brand new `Texture` that happens to get a
    recycled ID would find a stale cache entry from that ID's *previous* life already matching,
    so `generate_opengl()`'s `if(!is_bound()) bind()` guard incorrectly concluded it was already
    bound and skipped the real bind - silently uploading that texture's data to whatever was
    actually bound instead (the default texture object 0, since GL auto-unbinds a target when the
    bound texture is deleted). This is exactly what a second `Texture` test in the same process
    hit: `glGetTexLevelParameteriv` read back `0x0` instead of the image's real dimensions. Fixed
    by clearing any `boundTextures` entries pointing at an ID inside `cleanup()`, before deleting
    it.
  - `friend class TextureLoader;` isn't ported - that belonged to the old `util/asset_manager/`
    loader system, entirely superseded by this project's `AssetManager`.
  - Every `Texture` operation (constructor included) issues real GL calls, so unlike `Image` it
    can't be tested without a live context - the whole `texture.test.cpp` suite shares one hidden
    `RenderWindow` via `SetUpTestSuite`/`TearDownTestSuite` rather than creating one per test.
    Unlike `Window`/`RenderWindow` (thin plumbing with nothing to assert beyond "didn't throw"),
    `Texture` has real, checkable state, so it gets full test coverage against the real driver:
    handle allocation, bind/unbind unit tracking, real dimension/parameter readback via
    `glGetTexLevelParameteriv`/`glGetTexParameteriv`, a `set_image()` sub-region upload verified
    via `glGetTexImage`, and `TextureRegion::get_uv()`'s UV math.
- **`Shader`** (`shader.hpp`/`.cpp`) - a compiled+linked vertex/fragment GL program: construct
  from separate vertex+fragment `FileHandle`s (not reloadable) or a directory `FileHandle`
  holding `vertex.glsl`/`fragment.glsl` (reloadable via `reload()`), `bind()`/`unbind()`, a
  memoized `get_location()`, and a `set_uniform(name-or-location, value)` overload for every type
  in `Shader::Uniform` (int/uint/float/double/bool/`Vector{2,3,4}{i,u,f,d}`/`Matrix{2,3,4}`).
  Structurally unchanged except:
  - Added a small `get_shader_handle()` accessor (didn't exist in the old engine) - needed so
    tests can read uniform values back via `glGetUniform*` for real verification, the same reason
    `Texture::get_texture_handle()` already existed.
  - **Found a real bug**: both `set_uniform(..., const Vector4u&)` overloads (by name and by
    location) called `glUniform4ui(loc, value.x, value.y, value.w, value.w)` - `z` was never
    sent, `w` was sent twice. Every other `Vector4*` overload right next to it correctly passes
    `x, y, z, w`; comparing them made this an obvious copy-paste slip, same category as
    `Image::copy()`'s dropped `srcStride` earlier. Fixed, with a regression test reading all four
    components back via `glGetUniformuiv`.
  - Compile/link failure now throws `std::runtime_error` instead of the old
    `Logger::println(...); exit(0);` (same anti-pattern already fixed for `RenderWindow`'s GLAD
    failure). This surfaced a real follow-on issue: since the process no longer terminates on
    failure, the vertex/fragment/program GL objects allocated before the failure point would
    have leaked on every failed compile - `exit(0)` had made that moot by ending the process
    immediately. Now cleaned up (`glDeleteShader`/`glDeleteProgram`) on every failure path before
    throwing.
  - `Level::SEVERE` -> `LogLevel::Severe`, same translation as every prior port that logs.
  - Verified against a real hidden `RenderWindow` (same fixture pattern as `texture.test.cpp`):
    construction from a small hand-written GLSL pair (written to a real temp file, not reaching
    into Common's more complex embedded `assets/shaders/default/` pair, which uses SSBOs/
    instancing), `has_uniform`/`get_location`, `set_uniform` round-trips for `float`/`int`/
    `Vector3f`/`Matrix4` read back via `glGetUniformfv`/`glGetUniformiv`, the `Vector4u`
    regression above, and construction from deliberately-broken GLSL throwing instead of exiting.
- **`RenderState`** - pure data (depth/blend/cull/scissor/viewport/clear state), for the future
  `Renderer`'s state-diffing. Ported unchanged.
- **`Camera`/`PerspectiveCamera`/`OrthographicCamera`** (`camera.hpp`/`.cpp`) - position/
  orientation + view/projection matrices. Confirmed genuinely GL-free (the old file `#include`d
  `glad/gl.h` but never called anything from it - dropped; only `apply()` touches anything
  GL-adjacent, and that's just `Shader::set_uniform`). Added a `virtual` destructor (the old base
  class had pure-virtual `update_matrices()` but no virtual destructor - harmless today since
  neither subclass owns extra resources, but real undefined behavior waiting for whoever first
  deletes a `Camera*` pointing at a subclass) and `override` on both subclasses'
  `update_matrices()`. `project()`/`unproject()` are carried over exactly as they were - both
  skip a perspective divide by `w`, which is a no-op for `OrthographicCamera` (`w` is always 1)
  but would be wrong for a `PerspectiveCamera`; ported as-is since nothing demonstrates this
  actually breaks in practice, not fixed the way a confirmed bug would be - flagged as a
  known limitation instead.
- **`Clip`** - `glScissor` rect stack with hierarchical nesting via a static "current clip"
  pointer. No bugs found, ported unchanged.
- **`ShaderBuffer<T>`** (SSBO wrapper) - **found a real bug**: the old destructor's cleanup call
  was commented out entirely (`// glDeleteBuffers(1, buffer);`), and even uncommented it wouldn't
  have compiled (`buffer` is a `GLuint`, not a pointer - needs `&buffer`) - every instance leaked
  its GL buffer for the process's lifetime. Fixed, plus added a full rule-of-five (the old class
  had none at all - a bitwise copy would've `glDeleteBuffers`'d the same handle twice). Regression
  test confirms `glIsBuffer()` reports the handle as gone after destruction, which fails against
  the old body and passes with the fix.
- **`Mesh`** (data half) - pure CPU-side vertex/index/UV/color storage, ported unchanged.
- **`TexturePacker`** - CPU shelf-packs multiple `Image`s into one atlas, `create()` uploads the
  result to a `Texture`. Adapted to hold its texture in an `AssetSlot<Texture>` rather than a
  plain `unique_ptr<Texture>` - `get_region()` hands out a `TextureRegion`, which needs this
  project's pointer-based `Resource<Texture>` (the old engine's reference-wrapping `Resource<T>`
  could construct one directly from a `Texture&`; this project's can't), same adaptation already
  made for `Texture`/`TextureRegion` itself. **Found a real bug**: when the packer's canvas
  doubles mid-scan (not the upfront "does the first image even fit" check), the old code rebuilt
  its `mask` `Image` using the *stale, pre-resize* size before updating its own size-tracking
  variable - later bounds checks (which trust that variable) could then read past `mask`'s real,
  smaller buffer. Fixed by refreshing the size before rebuilding `mask`, not after. Regression
  test packs enough images to force a genuine mid-scan resize and confirms every image lands at
  its own distinct color with no cross-contamination.

Rendering Roadmap Phase 2 (`VertexArray`, `DrawableMesh`, `Framebuffer`, `AbstractRenderPass`/
`BufferedPass`) - see `refactor/rendering_roadmap.md`:

- **`VertexArray`** (`vertex_array.hpp`/`.cpp`) - the canonical GL buffer primitive going forward,
  replacing the old engine's separate `VertexBuffer` entirely (never ported - it's fully
  superseded and had a real bug of its own: a `static Buffer* tempBuffer` shared across every
  instance rather than per-instance). Owns one VAO plus N VBOs, each described by a
  `std::variant<StaticBuffer, DynamicBuffer>` carrying its own `BufferAttribute` list
  (index/type/count/stride/offset/divisor); `create()` allocates the GL objects and wires up
  attribute pointers, `set_data()` uploads (`glBufferData` for `STATIC`, `glBufferSubData` for
  `DYNAMIC`, clamped to the buffer's preallocated size). No bugs found - ported unchanged.
- **`DrawableMesh`** (in `mesh.hpp`/`.cpp`, alongside `Mesh` - same file pairing as the old
  engine) - rewritten against `VertexArray` instead of `VertexBuffer`. Builds 3 `StaticBuffer`s
  (positions at attribute 0, uv at 1, colors at 2 - matching
  `assets/shaders/mesh/vertex.glsl`'s layout) plus, when indexed, a 4th `StaticBuffer` on
  `GL_ELEMENT_ARRAY_BUFFER` with *no* attributes attached. That last part is a small, deliberate
  behavior change from switching primitives: the old `VertexBuffer`-based code routed the index
  buffer through the same per-attribute `buffer()` call used for real vertex attributes, which
  left a `glVertexAttribPointer(3, ...)`/`glEnableVertexAttribArray(3)` pair pointing at whatever
  `GL_ARRAY_BUFFER` happened to be bound (element buffers don't participate in vertex attribute
  state) - harmless since the mesh shader only declares locations 0-2, but not meaningful either.
  `VertexArray`'s per-buffer attribute list makes the "index buffer has zero attributes" case
  explicit instead of accidental, so this artifact just doesn't exist post-port; not tracked as a
  fixed bug since it had no observable effect. `render()`/copy semantics otherwise unchanged
  (copy ctor regenerates its own `VertexArray`, since `VertexArray` itself is non-copyable).
  `get_vertex_buffer()` renamed to `get_vertex_array()` to match the member's real type.
- **`Framebuffer`** (`frame_buffer.hpp`/`.cpp`) - owns one `Texture` per attachment (COLOR/DEPTH),
  in a `std::map<FramebufferAttachment, Texture>` it owns directly (no `Resource<T>`/`AssetSlot<T>`
  indirection needed here, unlike `Texture`/`TexturePacker`). `begin()`/`end()` maintain a
  "previous FBO" stack so passes can nest. **Fixed the same duplicate-key bug already found in
  `Texture`'s own defaults**: both the default COLOR and DEPTH `Attachment` blocks listed
  `{TEXTURE_WRAP_S, CLAMP_TO_EDGE}` twice instead of once for S and once for `TEXTURE_WRAP_T` -
  `std::map`'s initializer-list constructor keeps the first occurrence, so `TEXTURE_WRAP_T` was
  never actually set to `CLAMP_TO_EDGE` in either attachment (unlike `Texture`'s equivalent bug,
  GL's own default for `TEXTURE_WRAP_T` is `GL_REPEAT`, not `CLAMP_TO_EDGE`, so this one was a
  real, observable behavioral gap, not just a dead-code smell). Fixed in both attachment blocks;
  a regression test reverted the fix locally and confirmed it fails pre-fix
  (`glGetTexParameteriv` reading back `GL_REPEAT`) and passes post-fix. Also dropped the
  `ZoneScopedN("framebuffer_resize")` Tracy line from `resize()` (cross-cutting - Tracy isn't
  wired into engine-refactor anywhere).
- **`AbstractRenderPass`/`BufferedPass`** (`pipeline/render_pass.hpp`/`.cpp`, a new `pipeline/`
  subdirectory matching the old engine's layout - later roadmap phases add more files here) -
  `BufferedPass` owns a `RenderState`, a `Framebuffer` sized at construction, and a
  `Resource<Shader>`; `resize()` propagates to the framebuffer. Dropped the old file's
  forward-declared `Renderer`/`Scene` - neither is referenced anywhere in this specific file
  (dead here); can be re-added if a later phase's subclass actually needs them in this header.
  **Open design question, not resolved by this phase**: the old `GeometryPass`/`InterfacePass`
  called hardcoded `scene.render_world(...)`/`scene.render_interface(...)` virtual hooks - exactly
  the anti-pattern the ECS redesign already removed from `Scene` (see "ECS" below). How the render
  pipeline plugs into `SystemRegistry`/`Scene::render(dt)` is Phase 6's problem, not this one's.
Rendering Roadmap Phase 3 (`Material2D`/`Material3D`, `Collection`, `ShapePoint`/
`ShapeRenderType`, `SpriteProps`, `SpriteDrawCommand`/`ShapeDrawCommand`) - see
`refactor/rendering_roadmap.md`:

- **`Material2D`/`Material3D`** (`material.hpp`/`.cpp`) - `Material2D` is a lightweight shader/
  texture-pointer bundle (tint + arbitrary `unordered_map<string, Shader::Uniform>`), ported
  unchanged. `Material3D` is a PBR-ish 5-texture-slot material whose `apply()` binds a shared
  debug fallback texture (white/white/white/black/black) for any unset slot. Dropped the old
  engine's per-instance `Resource<Texture>` members (each initialized from a file-scope
  `StaticResource<Texture>` global) entirely in favor of 3 lazily-constructed, deliberately
  leaked `static Texture*`s (`new`'d, never `delete`d) - matching the old engine's own
  `StaticResource<T>` doc comment ("used ONLY for debug and other bullshit static defaulted
  resources... anything else will [leak]"). **Not** a plain `static Texture` value - that was
  the first attempt, and it's a real bug: a plain function-local static's destructor runs at
  process exit, after every GL context is already gone, and `glDeleteTextures` with nothing
  current segfaults (see "Known non-obvious fixes" below for the full story, found via a
  `coredumpctl`-confirmed crash). Verified the fallback really is shared (not re-allocated per
  `Material3D` instance) via a real-GL-context test: two independently-constructed `Material3D`s
  with unset slots resolve to the identical GL texture handle.
- **`Collection`** (new `batching/collection.hpp`/`.cpp`) - 3 debug textures + proj/trans/
  combined matrix trio + a dirty flag, `flush()` pure virtual. **The old engine's parallel
  `Batch` base class is not ported at all.** `git log --follow` on `engine/`'s
  `batching/batch.hpp` shows `Collection` was already a full replacement for `Batch` (commit
  `3f1471b`, "Working in a Collection as a replacement for batches"); `Batch` was kept alive
  afterward purely so `ShapeBatch`/`SpriteBatch` didn't have to change their public interface.
  Confirmed by reading both: `ShapeBatch`/`SpriteBatch`'s own `flush()`/`set_proj_matrix()`/
  `set_trans_matrix()` overrides bypass `Batch`'s logic entirely and forward straight to the
  `*Collection` versions, and a repo-wide search for `.begin()`/`.end()`/`.is_active()`/
  `set_blending()` turns up exactly one real caller of `Batch`'s begin/end interface:
  `phys_renderer_p.cpp`'s `PhysicsDebugRender`, a box2d debug visualizer - out of scope (Physics/
  box2d isn't wired into Runtime, see "Physics" below). `ShapeCollection`/`SpriteCollection`
  (Rendering Roadmap Phases 4/5) will inherit `Collection` directly - no `ShapeBatch`/
  `SpriteBatch` adapter layer, no diamond inheritance, no duplicate debug-texture copies. This
  also resolves the "duplicate default-shader singleton" cleanup the roadmap had flagged for
  those phases: with no `*Batch` class shadowing `*Collection`'s own default shader, there's
  nothing left to deduplicate once they land.
- **`ShapePoint`/`ShapeRenderType`** (new `batching/shape_point.hpp`), **`SpriteProps`** (new
  `batching/sprite_props.hpp`, needs `Material2D`), **`SpriteDrawCommand`/`ShapeDrawCommand`**
  (new `batching/draw_command.hpp`, needs both of the above) - pure data, no bugs found, ported
  as-is.
Rendering Roadmap Phase 4 (`ShapeCollection`) - see `refactor/rendering_roadmap.md`:

- **`ShapeCollection`** (new `batching/shape_collection.hpp`/`.cpp`) - immediate-mode 2D shape
  batcher: 8 draw primitives (`draw_polygon`/`draw_rect`/`draw_triangle` x2/`draw_circle`/
  `draw_line`/`draw_rect_line`/`draw_dotted_line`/`draw_arrow`) accumulate `ShapePoint`s into a
  queue of `ShapeDrawCommand`s, grouped so shader/z-layer/matrix state only re-uploads when it
  actually changed; `flush()` drains the queue into a single dynamically-sized `VertexArray`
  (position at attribute 0, color at 1, matching `ShapePoint`'s layout) via one `glDrawArrays`
  per command. No bugs found in the draw-primitive math itself - every change from the old
  engine is a mechanical `Resource<T>` API adaptation (engine-refactor's `Resource<T>` has no
  implicit conversion to `T*`/`T&`, unlike the old one):
  - `get_shader()` returns `*m_shader` instead of relying on an implicit `operator const T&()`.
  - `set_shader()`'s dirty-check compares `shader.get() != m_shader.get()` instead of relying on
    an implicit `operator T*()` for pointer inequality.
  - `new_command()`'s `ShapeDrawCommand` aggregate init passes `m_shader.get()` (a raw `Shader*`)
    instead of the `Resource<Shader>` itself.
  - **The default-shader static** (loads `assets/shaders/shapes`) is where Phase 3's deferred
    "how does a default `Resource<T>` argument work now that `StaticResource<T>` is gone" question
    actually gets answered: a private `static Resource<Shader> default_shader()` method wraps a
    lazily-constructed, deliberately leaked `static shared_ptr<AssetSlot<Shader>>*` (`new`'d,
    never `delete`d - same reasoning as `Material3D`'s fallback textures above, loading via
    `AssetFileSystem().open("assets/shaders/shapes")` - already-embedded per
    `draft_common-resources`) and is called as the constructor's default argument
    (`ShapeCollection(Resource<Shader> shader = default_shader())`) - legal C++, since default
    arguments are evaluated fresh at each call site, so this is cheap after the first real load.
    Verified two independently-constructed default `ShapeCollection`s resolve to the identical
    shader program handle (same reasoning as Phase 3's shared-fallback-texture test). The same
    pattern (including the leak-via-`new` requirement) will carry over to `SpriteCollection` in
    Phase 5.
  - `Logger::println(Level::WARNING, "Shape Batch", ...)` -> `LogLevel::Warning` + renamed the
    log tag to `"ShapeCollection"`, since "Shape Batch" now refers to a class this port doesn't
    have (see Phase 3's `Batch`-drop decision above).
Rendering Roadmap Phase 5 (`SpriteCollection`) - see `refactor/rendering_roadmap.md`:

- **`SpriteCollection`** (new `batching/sprite_collection.hpp`/`.cpp`) - instanced 2D sprite
  batcher: `draw()` routes each `SpriteProps` into an opaque `queue` or a z-sorted transparent
  `priority_queue` (translucent if `tint.a < 1`, the base texture's own format is transparent, or
  `material.transparent` is set); `flush()` runs opaque then transparent (toggling
  blend/depth-mask state between the two), and `flush_generic()` (a private template over a
  `QueueLike` concept, shared by both) batches same-material runs into chunks of up to
  `MAX_SPRITES_TO_RENDER` (1024) instances - one `ShaderBuffer<MatrixArray>` upload (SSBO
  binding 0, matching `assets/shaders/default`'s `Models` block) + one per-instance VBO upload +
  one `glDrawElementsInstanced` per chunk. No bugs found in the batching/flush logic itself -
  every change from the old engine is a mechanical adaptation:
  - **The 3 fallback textures** (base/normal/emissive, used when a sprite's material leaves them
    unset) need zero adaptation at all - `Collection` (Phase 3) already owns exactly these 3
    textures as protected members, so `SpriteCollection` just inherits them directly. This is a
    direct payoff of Phase 3's `Batch` removal: the old engine's `SpriteCollection` only had
    access to its *own* copy of these (via the `Batch`/`Collection` diamond `SpriteBatch`
    multiply-inherited from), which no longer exists here.
  - **The default-shader static**: unlike `ShapeCollection` (Phase 4), `SpriteCollection` has no
    persistent `Resource<Shader>` member of its own - the shader lives per-draw-call in
    `SpriteProps::material.shader` (a raw `Shader const*`, already ported in Phase 3). So the
    simpler adaptation applies here (same shape as `Material3D`'s debug-texture fallbacks, not
    `ShapeCollection`'s `Resource<Shader>`-returning static method): an anonymous-namespace
    `Shader& default_shader()` backed by a **leaked** `static Shader* = new Shader(
    AssetFileSystem().open("assets/shaders/default"))`. Applying the Phase-4 lesson (below)
    correctly from the start this time - verified via `coredumpctl list` after a full test run
    showing no new crash, in addition to the usual shared-instance test.
  - Dropped the dead `#include "draft/rendering/vertex_buffer.hpp"` (unused even in the old file
    - `VertexBuffer` itself isn't ported at all, per Phase 2's decision).
Rendering Roadmap Phase 6 (`Renderer`, `GeometryPass`, `CompositePass`) - see
`refactor/rendering_roadmap.md`:

- **The pass pipeline no longer knows about `Scene`/ECS at all - a real redesign, not a 1:1
  port.** The old `GeometryPass`/`InterfacePass` called `scene.render_world(renderer, dt)`/
  `scene.render_interface(renderer, dt)` - virtual hooks on the old `Scene`, exactly the
  hardcoded-render-phases anti-pattern the ECS redesign already rejected (see "ECS" below).
  engine-refactor's `Scene` has no such hooks, only `update(dt)`/`render(dt)` running every
  registered `AbstractSystem`'s matching cadence. Resolution: **submission and flushing are now two
  separate steps.** "Submit this frame's geometry" becomes an ordinary `AbstractSystem::render(dt)` call
  (a future `RenderSystem`/`InterfaceSystem`, Rendering Roadmap Phase 9, holding a `Renderer&`
  and calling `renderer.shape.draw_*()`/`renderer.batch.draw()` - the same pattern every other
  system already uses) - by the time any pass runs, `renderer.batch`/`renderer.shape` are already
  populated for the frame, and a pass's only job is to flush them. Concretely:
  - `GeometryPass::run()` dropped both its `Scene&` and `Time deltaTime` parameters - it's now
    `const Texture& run(Renderer& renderer)`. It does nothing time- or Scene-dependent itself.
  - `Renderer`'s pure-virtual `render_frame` dropped `Scene&`, becoming
    `virtual void render_frame(Time deltaTime) = 0;` - the once-per-frame pass-pipeline entry
    point a concrete renderer (Phase 9's `DefaultRenderer`) implements, called by the application
    main loop *after* `scene.render(dt)` already ticked every per-frame `AbstractSystem`. `Renderer`
    needs no `Scene` forward declaration at all now.
  - `CompositePass` needed no change - it never took `Scene&` in the old engine either.
  - The same shape change applies to Phase 9's `InterfacePass` when it lands.
- **`Renderer`** (`pipeline/renderer.hpp`/`.cpp`) - owns the shared `SpriteCollection batch`/
  `ShapeCollection shape` every pass flushes from, `begin_pass`/`end_pass` (single-current-pass
  tracking), a fullscreen quad, and `set_state()` (GL state-diffing so passes don't redundantly
  re-set state that's already current). **Found and fixed two real bugs while testing against
  real GL state readback** (beyond the two already flagged in the roadmap survey - a cull-face
  operator-precedence readability slip and mislabeled fullscreen-quad-vertex-winding comments,
  both fixed as originally planned, no behavior change):
  - **Scissor: a real crash.** `if(newState.scissor && (force || newState.scissor->x !=
    m_previousState.scissor->x || ...))` unconditionally dereferences
    `m_previousState.scissor->x` once the `||` chain reaches it - if `newState.scissor` is set but
    `m_previousState.scissor` isn't (e.g. the first scissor rect ever applied without
    `force=true`), that's a disengaged `std::optional` dereference. Not hypothetical - a hardened
    libstdc++ build aborts on it, caught by a real test. Fixed by treating "no previous scissor"
    the same as `force` (`!m_previousState.scissor ||`).
  - **Cull mode: a real staleness bug, not just the flagged readability issue.** The original
    `if(force || newState.cullFace && newState.cullMode != m_previousState.cullMode)` (parens
    added per the roadmap's own flag) still has a deeper problem: `m_previousState = newState`
    unconditionally overwrites *every* field at the end of `set_state()`, including `cullMode`,
    even when culling was disabled and `glCullFace` was therefore never actually called. That
    lets `m_previousState.cullMode` silently diverge from the real `GL_CULL_FACE_MODE` - so a
    later re-enable-with-a-different-mode call can wrongly conclude "unchanged" and skip
    reissuing `glCullFace`, leaving the wrong face culled. A real GL-state-readback test
    (enable+mode A, disable+mode B, re-enable+mode B) reproduced this exactly. Fixed by dropping
    the mode-diffing entirely for this one field - `glCullFace` is cheap, so always reissuing it
    whenever culling is (or becomes) enabled trades a negligible redundant call for guaranteed
    correctness.
- **`GeometryPass`** (`pipeline/passes/geometry_pass.hpp`/`.cpp`) - binds its own `Framebuffer`
  (inherited via `BufferedPass`, Phase 2) and flushes `renderer.batch`/`renderer.shape` (opaque,
  then transparent) into it. Keeps the old code's `RenderState& m_opaqueState = p_state;`
  reference-aliasing trick unchanged - the "opaque" state literally *is* `BufferedPass::p_state`,
  not a second copy; a real intentional design, not an accident, so it's commented rather than
  "cleaned up."
- **`CompositePass`** (`pipeline/passes/composite_pass.hpp`/`.cpp`) - blits a texture (typically
  `GeometryPass`'s output) via the fullscreen quad. Ported unchanged.
- **A real, discovered limitation of the "leaked, process-lifetime" default-shader/texture
  pattern** (`Material3D`'s fallbacks, `ShapeCollection`/`SpriteCollection`'s default shaders -
  Phases 3-5): a leaked GL object is only valid within the specific GL context that first
  constructed it lazily. A real single-window application never notices this (one context lives
  for the process), but this test suite creates a fresh, unshared `RenderWindow`/context per test
  *fixture* - so a later fixture reusing an earlier fixture's already-leaked default shader (whose
  originating context/window has since been torn down by that earlier fixture's
  `TearDownTestSuite`) hits a real `GL_INVALID_VALUE` from `glUseProgram` on a now-foreign program
  ID. Found via `GeometryPass`'s tests (the first tests to reuse `ShapeCollection`'s leaked
  default *across* fixtures rather than within one). Not a production bug - fixed at the test
  level (each fixture gives `renderer.shape` its own fixture-owned shader via `set_shader()`
  rather than relying on the cross-fixture-fragile default) - but worth recording here as a real
  characteristic of the leak-forever pattern, not just its already-documented crash-at-exit
  hazard.
Rendering Roadmap Phase 7 (`Font`, `TextRenderer`) - see `refactor/rendering_roadmap.md`. The
first genuinely new external dependency in Rendering: Freetype, via `find_package(Freetype
REQUIRED)` (system `freetype2`, matching the old engine's own approach - its vendored
`engine/vendor/freetype/` was already dead, never built), linked `PRIVATE` to `draft_runtime`.

- **`Font`** (`font.hpp`/`.cpp`) - pImpl-wrapped `FT_Library`/`FT_Face`; lazily bakes glyphs into
  `GREYSCALE` texture atlas pages (2048x2048, growing to a new page on overflow) per font size,
  bakes ASCII 0-127 up front for the default size. **Found and fixed 3 real bugs in
  `bake_glyph()`'s packing** - more than the 2 the roadmap survey had already flagged, all
  confirmed via a real test that bakes enough glyphs to force actual page rollover (not just
  reasoned about):
  - The already-flagged **dangling-reference bug**: `baseImage`/`baseTexture` were bound as
    references *before* a possible `fontType.images.emplace_back(...)` a few lines later (when a
    glyph needs a new page); `emplace_back` can reallocate the vector, and the old code then
    *assigned through* the now-dangling `baseImage` reference instead of rebinding (references
    can't rebind) - not hypothetical UB: temporarily reverting just this part (keeping the other
    fixes below) reliably crashed with `free(): invalid pointer`, real heap corruption. Fixed by
    not fetching `baseImage`/`baseTexture` until *after* every vector mutation for that glyph is
    done.
  - **Newly found while testing the above**: `previousGlyphBounds` (used to compute where the
    *next* glyph goes) was recorded from `bounds` *before* the row-wrap/page-wrap adjustments
    that same call makes to `bounds` - so once a wrap actually happened, every subsequent glyph
    inherited a stale x/y that never accounted for the wrap, eventually placing glyphs at y
    coordinates that walked straight past the atlas page's real 2048px height without ever
    tripping the "shift to a new texture" check meant to catch exactly that. Reproduced directly:
    baking enough glyphs to force a wrap left several glyphs' `TextureRegion` bounds exceeding
    their own texture's real reported size. Fixed by recording `previousGlyphBounds` *after* both
    wrap adjustments, using the bounds actually used to place the glyph.
  - The already-flagged **duplicate `TEXTURE_WRAP_S` key** in `FONT_TEXTURE_PROPS` - same shape,
    same fix as `Texture`'s/`Framebuffer`'s own already-fixed defaults.
  - `exit(0)` on Freetype init/load failure replaced with `throw std::runtime_error(...)`,
    matching the exception-first fixes already applied to `RenderWindow`/`Shader`.
  - `FontType::textures` changed from `vector<shared_ptr<Texture>>` to
    `vector<shared_ptr<AssetSlot<Texture>>>` - the same `AssetSlot<T>`-wrapping idiom `TexturePacker`
    (Phase 1) already established - so each glyph's `TextureRegion` can hold a real
    `Resource<Texture>` instead of relying on the old engine's implicit reference-to-`Resource<T>`
    conversion.
  - Dropped the dead `#define STB_TRUETYPE_IMPLEMENTATION` and `#include "stb_image_write.h"` -
    `Font` doesn't call any `stbtt_*`/`stbi_write_*` function; both were leftover dead code.
- **`TextRenderer`** (`batching/text_renderer.hpp`/`.cpp`) - `TextProperties` (pure data, ported
  as-is), `get_text_bounds()`, `draw_text()` (x2 overloads). Takes `SpriteCollection&` instead of
  the old `SpriteBatch&` (never ported - dropped in Phase 3/5's `Batch`-removal decision).
  `defaultFont`/`defaultShader` (old `StaticResource<T>`) became a leaked `static Font*` and the
  established `Resource<Shader>`-returning `default_shader()` pattern (`TextRenderer` stores a
  persistent `Resource<Shader> fontShader` member, so - like `ShapeCollection`, not
  `SpriteCollection`/`Material3D` - it needs the `AssetSlot`-wrapped flavor, not a bare
  reference). **Found and fixed a real X/Y asymmetry bug** in the vertical glyph-placement math
  (the roadmap had flagged this as "double-check before porting verbatim," not yet confirmed
  either way): X only scales the glyph-intrinsic `bearing.x` offset, treating the caller-supplied
  `position.x` as already being in final space (`currX` accumulates from already-scaled
  advances, never re-scaling the position); Y scaled the *entire* expression, including the
  caller's `position.y` - `(props.position.y - glyph.bearing.y) * props.scale + size.y`. No
  top-left-anchor convention explains scaling the position on one axis but not the other. Fixed
  to match X: `props.position.y - glyph.bearing.y * props.scale + size.y`. Extracted the per-glyph
  position math into a small, pure `static Vector2f compute_glyph_top_left(...)` method so the
  fix is directly unit-testable with plain numbers (no GL/Freetype/Font needed) rather than a
  fragile pixel-readback test.
Rendering Roadmap Phase 8 (`ParticleSystem`, `Animation`) - see `refactor/rendering_roadmap.md`.
Both are pure CPU-side logic (their GL dependencies - `Texture`, `SpriteCollection`,
`Material2D` - were already ported); close reading found more bugs in both files than the
roadmap's own prior survey had flagged.

- **`ParticleSystem`** (`particle_system.hpp`/`.cpp`) - a fixed-size, ring-buffer-recycled CPU
  particle pool. **Found and fixed 4 real bugs in `render()`/`emit()`**, 3 more than the 1
  originally flagged:
  - The flagged **inverted begin/end interpolation**: `life = lifeRemaining / lifeTime` is 1 at
    birth, 0 at death (life *remaining*, not elapsed). The old code interpolated directly on
    `life`, which evaluates to the *end* values at birth and the *begin* values at death -
    backwards from the field names. Fixed by interpolating on `age = 1 - life` instead. Extracted
    into a pure `static VisualState compute_visual_state(...)` method (same shape as Phase 7's
    `TextRenderer::compute_glyph_top_left`) so the direction fix is directly unit-testable with
    plain numbers.
  - **Newly found: the interpolated size was computed and then never used.** `render()` computed
    `size` via the lerp, then built the `SpriteProps` using `particle.size` (the *static* value
    set once at `emit()` time) instead - the interpolated value was silently discarded, so
    particles never actually visually resized over their lifetime. Fixed to use the computed
    value.
  - **Newly found: the interpolated color was computed and then never applied.** `particleMaterial`
    only ever set `.baseTexture` - `.tint` was never assigned, so every particle rendered in the
    material's default white/opaque tint regardless of `colorBegin`/`colorEnd`/the alpha fade.
    Fixed with `particleMaterial.tint = visual.color;` (no need to also force
    `.transparent = true` - `Material2D`'s own `tint.a < 1.f` check already routes a fading
    particle to the transparent queue automatically once alpha drops below 1).
  - **Newly found: a broken decrement-with-wraparound idiom.** `poolIndex = --poolIndex %
    particlePool.size();` decrements the unsigned `poolIndex` *before* the modulo - from
    `poolIndex == 0` this underflows to `SIZE_MAX` first, and `SIZE_MAX % size` is not generally
    `size - 1` (615, not 999, for `size=1000`). Always stayed in-bounds (modulo guarantees that),
    so this was a minor recycling-order quirk rather than a crash - fixed anyway (extracted into
    a pure `static recycle_index(...)` too) since the correct idiom is no more complex.
  - Adapted `render(SpriteBatch&)` to `render(SpriteCollection&)` (`SpriteBatch` was never
    ported). `ParticleProps::texture`/new `ParticleProps::shader` became plain nullable
    `Texture*`/`Shader*` (nullptr means "use `SpriteCollection`'s own shared fallback") instead of
    the old `Resource<Texture>` defaulted from a `StaticResource<Texture>` pointed at
    `assets/textures/particles/circle.png` - **confirmed that file doesn't exist anywhere in this
    repository**, not even in the old `engine/`'s own assets tree, so porting the literal default
    forward would just carry a dead reference. `SpriteCollection::draw()` already has its own
    fallback-texture/fallback-shader resolution for exactly this "unset" case, so `ParticleSystem`
    doesn't need a fallback of its own at all.
- **`Animation`** (`animation.hpp`/`.cpp`) - a sprite-sheet animation (fixed texture + a list of
  frame regions/durations parsed from an Aseprite/TexturePacker-shaped JSON manifest).
  - The flagged **hard `Assets&` dependency**: the old constructor took `Assets& assets` directly
    and resolved `meta.image` into a texture path via the old asset manager, which doesn't exist
    in engine-refactor. Reworked per the roadmap's own plan: the constructor now takes a
    `Resource<Texture>` directly from the caller
    (`Animation(Resource<Texture> texture, const FileHandle& handle)`) - `meta.image`-driven path
    resolution is dropped entirely, since the caller already decided which texture to use. This
    also let the old placeholder-texture member (relying on the old `Resource<T>`'s
    reference-taking convenience constructor, which doesn't exist here) drop out - the new
    `Resource<Texture> texture` member is directly initialized from the constructor parameter.
  - The flagged **`get_frame()` magic-number fallback, confirmed as a real out-of-bounds read**:
    the fallback path `frames[(int)(frameTime / 100) % 20]` only runs when the main loop falls
    through without a match - which happens whenever `totalFrameTime <= 0` (an empty or
    all-zero-duration animation), since `fmodf(frameTime, 0)` produces NaN and every
    `frameTimes[i] > frameTime` comparison against NaN is false. `% 20` has no relationship to
    the real `frames.size()` - reads out of bounds whenever there are fewer than 20 frames, the
    overwhelmingly common case. Fixed: an empty animation now throws from `get_frame()` (matching
    this port's exception-first precedent) instead of hitting undefined behavior converting NaN
    to `int`; the truly-defensive fallback (a non-empty, malformed animation whose loop still
    falls through) returns `frames.back()` instead of an unrelated magic-number index. Verified
    both paths with real tests, including an all-zero-duration-but-non-empty animation that
    exercises the fallback without the empty-animation guard hiding it.
  - `JSON data(handle);` (Common's `JSON` already has a `FileHandle`-parsing constructor, simpler
    than the old `JSON::parse(handle.read_string())`). `get_frame()` is now `const`.
Rendering Roadmap Phase 9 (`InterfacePass`, `RenderSystem`, `DefaultRenderer`) - see
`refactor/rendering_roadmap.md`. Closes out the pass pipeline started in Phase 6.

- **`InterfacePass`** (`pipeline/passes/interface_pass.hpp`/`.cpp`) - structurally a clone of
  `GeometryPass`: no framebuffer of its own, draws directly onto whatever's currently bound.
  Dropped `Scene&`/`Time` from `run()` the same way `GeometryPass::run()` did in Phase 6 -
  submission already happened via a per-frame `AbstractSystem` before the pass pipeline runs. **Found and
  fixed a real bug via structural-sibling comparison** (the same method that caught Phase 6's
  cull-face staleness): `m_transparentState.depthWrite = true;` is a literal no-op against
  `RenderState`'s own default (`depthWrite = true`), while `GeometryPass`'s analogous transparent
  state explicitly sets `depthWrite = false` - the documented-correct convention so transparent
  geometry doesn't occlude other transparent geometry behind it. Nothing justifies interface
  transparency needing depth writes when geometry transparency explicitly doesn't; reads as a
  copy/paste `true`/`false` slip. Fixed to `false`, matching `GeometryPass`. Regression-tested by
  reading back `GL_DEPTH_WRITEMASK` immediately after a transparent-only `run()`.
- **`SpriteComponent`** (`components/sprite_component.hpp`, new) - a porting gap found along the
  way: `RenderSystem` has no meaning without it, but it was never ported (only
  `TransformComponent`/`TagComponent`/`RigidBody*`/`Joint*`/`Audio*` existed). Small, reflectable,
  pure data (`TextureRegion texture`, `Vector2f size`/`origin`, `float zIndex`), plus a nullable
  `Shader* shader = nullptr` override - not in the old engine's `SpriteComponent`, added for the
  same reason Phase 8 added `ParticleProps::shader`: it lets a test give a submitted sprite a
  shader owned by its own GL context instead of racing `SpriteCollection`'s process-lifetime
  leaked default (only valid within whichever context first constructs it). Default behavior for
  a real app is unchanged - `nullptr` still resolves to `SpriteCollection`'s own default, exactly
  like the old engine.
- **`RenderSystem`** (`ecs/render_system.hpp`/`.cpp`, moved from the old `draft/systems/` to
  `draft/ecs/` alongside `AudioSystem`) - reworked from a `Scene&`-constructed class exposing a
  manually-called `render(SpriteCollection&)` method into an ordinary `AbstractSystem` (constructor takes
  `Registry&` + `Renderer&` directly, per the design Phase 6 already flagged as the resolved
  shape), overriding `render(Time dt)` to submit every `<SpriteComponent, TransformComponent>`
  entity into `rendererRef.batch` - registered via `scene.get_systems().add<RenderSystem>(...)`
  like any other system, not a hardcoded `Scene::render_world` override. Dropped the old
  `RenderWindow&` member (confirmed genuinely unused - only `registryRef` was ever touched) and
  the Tracy `ZoneScopedN` line (cross-cutting finding, stripped on sight).
- **`DefaultRenderer`** (added to the existing `pipeline/renderer.hpp`/`.cpp`, alongside the
  already-ported `Renderer` base) - wires geometry, composite, and interface passes together. The
  design question Phase 6 left open ("Phase 9 must follow this same shape") was already resolved
  by Phase 6's own work: `Renderer::render_frame(Time)` takes no `Scene&`, so
  `DefaultRenderer::render_frame` is a pure pass-pipeline runner (geometry -> composite the result
  -> interface) with zero `Scene&` involvement; the caller's main loop calls `scene.render(dt)`
  (ticking `RenderSystem` among others) before `renderer.render_frame(dt)`. Its three shaders load
  via `AssetFileSystem().open("assets/shaders/{geometry,composite,default}")` - the same pattern
  `ShapeCollection::default_shader()` already established. No bugs found in `DefaultRenderer`'s own
  logic.
- **A real test-suite ordering hazard, found and fixed**: adding `RenderSystemTest` initially
  broke 4 previously-passing `SpriteCollectionTest` tests with a real `GL_INVALID_VALUE` (1281) -
  not a false alarm. Both suites' `SpriteCollection`s fall back to the same process-lifetime
  leaked default shader when a draw's material leaves `.shader` unset; whichever test file's
  translation unit happens to link/register first "wins" construction of that shared static under
  its own (soon-destroyed) `RenderWindow` context, invalidating it for every other suite that
  reuses it under a different context later in the same process. Not a new bug in
  `SpriteCollection` (real single-window apps never hit this - the leaked default lives for the
  process's one real context) - fixed by giving `RenderSystemTest` its sprite entity an explicit,
  fixture-owned shader via the new `SpriteComponent::shader` override above, the same way every
  other pipeline test already avoids this shared static.
- **Deliberately not ported yet**: Tracy GPU profiling (not wired into engine-refactor), ImGui/
  RmlUi integration. See `refactor/rendering_roadmap.md` for the full phased plan. `Keyboard`/
  `Mouse` are ported too now - see "Input" below.

Rendering Roadmap Phase 10 (`Model`, glTF) - see `refactor/rendering_roadmap.md`. This closes out
the entire Rendering roadmap. Per the roadmap's own framing this was a **redesign, not a port** -
"the single buggiest file surveyed" - and close reading confirmed that: all 7 originally-flagged
bugs were real, plus 2 more found while fixing the node-transform gap.

- **New external dependency**: tinygltf (`vendor/tinygltf/`, a single vendored `tiny_gltf.h`,
  matching `stb`'s existing vendoring shape - an `INTERFACE` CMake target, linked `PRIVATE` to
  `draft_runtime`). Reconciled with already-vendored `stb`/`draft_common`'s `nlohmann_json` via
  tinygltf's own `TINYGLTF_NO_INCLUDE_JSON`/`_STB_IMAGE`/`_STB_IMAGE_WRITE` macros rather than
  letting it bundle its own copies - `TINYGLTF_IMPLEMENTATION` is defined in `model.cpp` itself
  (the sole consumer), unlike `stb`'s shared dedicated impl TU.
- **Indices always read as `unsigned short`, confirmed real.** `component_byte_size()` existed but
  was dead code - the index buffer was always reinterpreted as `unsigned short` regardless of the
  accessor's real `componentType`. Any glTF using 32-bit indices (any mesh over 65535 vertices)
  silently parsed garbage. Fixed by extracting a pure `static Model::read_indices(...)` helper
  that branches on the real component type (byte/short/int, glTF indices are always unsigned) -
  directly unit-tested with synthetic byte buffers, no glTF file needed.
- **Missing existence checks before indexing, confirmed real.**
  `primitive.attributes["TEXCOORD_0"]` and `primitive.indices` are both optional per the glTF
  spec (`std::map::operator[]` on a missing `"TEXCOORD_0"` key default-inserts and silently reads
  accessor index 0). Fixed with explicit existence/`>= 0` checks before touching either.
- **Mesh/primitive index-space mismatch, confirmed real.** `load_meshes()` flattened every
  primitive into one entry per *primitive* across all meshes, but `load_nodes()` indexed
  `matrices[node.mesh]` - a *mesh* index into an array actually keyed by *primitive*. Wrong the
  moment any mesh has more than one primitive. Fixed by tracking a `[start, end)` primitive range
  per glTF mesh index, so a node's transform applies to every primitive in its mesh's range -
  regression-tested with a real 2-primitive-mesh fixture confirming both primitives get the same
  real transform, not just one.
- **No hierarchical transform accumulation at all, confirmed real.** `load_nodes()` set each
  node's matrix directly from identity with zero parent/child composition - `node.children` was
  never read. Fixed with a DFS from each scene's root nodes (or "any node no one lists as a
  child," if there's no scene), composing `worldMatrix = parentWorld * localMatrix` down the tree
  - regression-tested with a real parent/child fixture confirming the child's world matrix
  reflects *both* levels' translations, not just its own local one.
- **Newly found while fixing the above: `node.rotation` was treated as Euler angles, not a
  quaternion.** glTF's spec is unambiguous - `node.rotation` is always `[x, y, z, w]`, a
  normalized quaternion. The old code fed `rotation[0..2]` through three chained per-axis
  `Math::rotate()` calls as if they were radian angles, and never read `rotation[3]` (`w`) at all
  - wrong orientation for *any* rotated node in *any* real-world glTF file, not a hypothetical.
  Fixed with a real `Quaternion{w, x, y, z}` composed via `Math::mat4_cast(...)` - regression-
  tested by rotating a point 90 degrees about +Z and confirming the result lands exactly where a
  real quaternion rotation places it, not where the old fake-Euler chain would.
- **Newly found while fixing the above: `node.matrix` was never checked.** glTF nodes specify
  *either* a raw 4x4 `matrix` *or* T/R/S, never both - a node authored with a baked `matrix`
  (common from some export tools) silently rendered with an identity transform, since none of the
  T/R/S `if(...size() > 0)` branches would ever trigger. Fixed by checking `matrix.size() == 16`
  first and using it directly, only falling back to T/R/S when absent - regression-tested
  confirming `matrix` wins even when (malformed) T/R/S values are also present.
- **Copy ctor/assignment dangle, confirmed real, with a design fork.** `materials` holds raw
  `Texture*` observers into `embeddedTextures`, but the copy ctor/assignment never copied
  `embeddedTextures` - dangling the instant the source was destroyed. **`Texture` is non-copyable
  in engine-refactor** (`Texture(const Texture&) = delete`), so a true deep-copy isn't even
  possible, and reloading from GL on every `Model` copy would be both expensive and semantically
  wrong (two `Model`s from the same glTF file reasonably share one GPU resource, not duplicate
  it). Fixed by changing `embeddedTextures` to `vector<shared_ptr<Texture>>` - copies share
  ownership of the same live textures - regression-tested with a real GL fixture: copy a loaded
  `Model`, destroy the original, render from the copy, confirm no GL error.
- **`reload()`/`load()` never cleared `embeddedTextures`, confirmed real.** Repeated reloads
  accumulated duplicate textures forever. Fixed by clearing `embeddedTextures` in `load()`
  alongside its existing `materials.clear()`/`meshes.clear()` - safe now that the `shared_ptr`
  change above means any *other* `Model` holding a copy keeps its own textures alive
  independently. Regression-tested calling `reload()` twice and confirming the embedded-texture
  count stays constant.
- **`reload_materials()` dereferenced `handle` unconditionally, confirmed real** - unlike
  `reload()`, which correctly short-circuits via `!reloadable ||`. A procedurally-built `Model`
  (no `handle`) calling `reload_materials()` was UB. Fixed with the same guard `reload()` already
  had.
- **Also fixed alongside the node-transform work**: the `basePath` passed to tinygltf's ASCII
  loader was hardcoded to `"assets"` regardless of where the model file actually lives, so
  external buffer/image URIs only resolved correctly for models literally placed at the top of
  `assets/`. Now derived from the handle's own parent directory instead (`.glb`, self-contained,
  is unaffected either way). Uses `std::filesystem::path::parent_path()` directly rather than
  `FileHandle::parent()`, which throws on an empty parent path (e.g. a bare filename with no
  directory component) - found while writing this phase's own test fixtures, which use exactly
  that shape.
- `Model` now holds `vector<DrawableMesh>` (Phase 2) instead of the old `vector<Mesh>` +
  `vector<unique_ptr<VertexBuffer>>` + a manual `buffer_meshes()` step (`VertexBuffer` itself was
  never ported - Phase 2 dropped it entirely in favor of `VertexArray`). `render()` calls
  `material.apply(shader)` + `shader.set_uniform("model", ...)` before `drawableMesh.render()`
  (material application doesn't need the VAO bound). Dropped the `avoidGL` constructor/`load()`
  parameter - confirmed dead, accepted but never referenced anywhere in the old `load()`'s body.
  `exit(0)` on load failure replaced with `throw std::runtime_error(...)`, matching this port's
  exception-first precedent everywhere else.
- **A real test-suite ordering hazard, found twice more while testing** - the same class of issue
  Phase 9 already found for `SpriteCollection`'s leaked default shader, this time for
  `Material3D`'s own leaked debug fallback textures (`debug_white()`/`debug_black()`/
  `empty_normal_map()`, `material.cpp`). Any `Model` primitive with a material that leaves a
  texture slot null (including the built-in "missing_material_draft" dummy used for
  `primitive.material == -1`) falls back to those statics - `material.test.cpp` already claims
  them under its own context earlier in this binary, so a naive `Model` test rendering with any
  null texture slot hit a real `GL_INVALID_OPERATION` (1282) when run as part of the full suite.
  Not a new bug (real single-window apps never hit this). Fixed at the test level: fixtures that
  actually call `render()` populate every material texture slot explicitly; the fixture that
  intentionally exercises the no-material dummy path only checks parsing, not rendering.
- Private, pure-math helpers (`compute_local_matrix`, `read_indices`) follow the same
  friend-test-accessor pattern established in Phases 8-9 (`ParticleSystemTestAccess`,
  `TextRendererTestAccess`) - a `ModelTestAccess` struct is the sole declared friend. Unlike those
  two, the functions they wrap are `static` members of `Model` rather than anonymous-namespace
  free functions in `model.cpp`, specifically so they can call each other without a second friend
  declaration; `tinygltf::Model` is forward-declared (`namespace tinygltf { class Model; }`) in
  `model.hpp` so the real `<tiny_gltf.h>` never leaks into the public header despite the loading
  orchestration itself also being private `static` members.

### ECS
`draft/ecs/` (`Entity`, `Registry`, `Scene`, `SystemRegistry`, `RelationshipSystem`) is a
redesign of the old engine's `core/` - a real critique-then-rework, not a mechanical port (the
old `Scene` was never loved, and a review of it plus its two real consumers, `editor_scene.cpp`
and `test_scene.cpp`, backed that up with specifics).

- **`Scene` is now a pure ECS container** - an entity registry, entity creation, parent/child
  relationships, and whichever systems are registered against it. The old `Scene` also owned
  rendering (`render`/`render_world`/`render_interface`), input routing (`handle_event`), and
  app-lifecycle hooks (`on_attach`/`on_detach`), which meant it had to `#include` the rendering
  pipeline and input event headers just to exist, and meant a scene could only be authored as a
  hand-written, compiled C++ subclass (`EditorScene : public Scene`, `TestScene : public Scene`)
  - directly at odds with architecture.md's "Scenes are simply another asset type" and the
    editor-driven, loose-JSON-scene workflow the rest of the roadmap commits to. None of that
  render/input/lifecycle surface exists on the new `Scene` - there are no virtuals left to
  override; whatever eventually drives the game loop once Rendering/Input exist in Runtime
  should hold and drive a `Scene`, not extend it.
- **`SystemRegistry` replaces two incompatible old patterns with one.** `test_scene.cpp` held a
  `PhysicsSystem` as a plain member and called `.update()` on it by hand inside its own
  `update()` override; `editor_scene.cpp` instead stashed one in EnTT's type-erased
  `registry.ctx()` and fished it back out later - two different ways to attach the same kind of
  thing to a scene, chosen ad hoc per subclass, with nothing driving a consistent per-tick
  order. `SystemRegistry::add<T>(Args&&...)` (construction args forwarded, same convention as
  `Entity::add_component`) is the one mechanism now. `RelationshipSystem` stays a dedicated
  `Scene` member rather than going through `SystemRegistry` - it reacts to component
  construction/destruction, it has no per-tick work of its own to schedule.
- **`AbstractSystem` has two cadences, not one: `update(dt)` and `render(dt)`.** The old engine ran
  `Scene::update()` from a fixed-timestep accumulator (for deterministic physics) and
  `Scene::render()` once per frame with the actual variable frame delta, regardless of how many
  (or how few) fixed steps just ran. That's a real, load-bearing distinction - unlike the
  `render`/`render_world`/`render_interface` split above, which was just two arbitrary drawing
  phases hardcoded as separate methods for no structural reason - so it's kept, just at the
  `AbstractSystem`/`SystemRegistry` level instead of hardcoded onto `Scene`: `update()` and `render()`
  both default to a no-op, so a system only overrides whichever cadence(s) it actually needs (a
  physics system overrides `update()` only; a rendering or interface/UI system overrides
  `render()` only - it's an ordinary system like any other, not a special hook, per the
  discussion that led here). `SystemRegistry::update_all(dt)`/`render_all(dt)` (and
  `Scene::update(dt)`/`render(dt)`, which just forward to them) call the respective hook on
  every registered system, in registration order. A system needing a draw target takes it as a
  constructor argument through `add<T>(Args&&...)`, same as any other dependency - `render(dt)`
  deliberately doesn't carry one itself.
- **`Entity::operator==` now compares which scene a handle belongs to, not just the raw ID.**
  `entt::entity` values are small, recycled, *per-registry* integers - the old operator only
  compared that integer, so two entities from different scenes could (and, per a new regression
  test, immediately do for the first entity created in any two scenes) compare equal despite
  being in different worlds entirely.
- **Component accessors now consistently assert `is_valid()` in debug builds.** The old
  `try_get_component()` alone guarded against an invalid handle (`m_context == nullptr` or a
  null entity ID); `get_component`/`add_component`/`has_component`/`remove_component` had no
  guard at all, in *either* build configuration - an immediate null-deref on a stale or
  default-constructed handle. They're assert-guarded now (consistent with architecture.md
  keeping runtime systems, ECS included, exception-free during normal execution - this is a
  debug-time contract, not a thrown error), while `try_get_component()` keeps its original,
  additional guarantee of being safe in every build configuration, for the specific "this handle
  might be stale" case it exists for.
- **`Entity::is_valid()` now actually asks the registry**, not just its own fields. It used to
  be `m_context != nullptr && m_entityID != entt::null` alone, which only reflects reality for
  the *exact* `Entity` object `destroy()` was called through - every other copy of that same
  handle (sitting in a `ParentComponent::children` vector, a UI's "selected entity", ...) would
  report valid forever, even long after the underlying entity was destroyed some other way. It
  now also checks `registry.valid(m_entityID)`.
- **`NULL_ENTITY` stays**, even though a default-constructed `Entity{}` is already equivalent -
  it reads better at a comparison/default-value call site, and removing it wasn't asked for.
- **Two real bugs found in `RelationshipSystem` via new tests that didn't exist for it before**
  (both present verbatim in the pre-refactor `engine/`):
  - Backfilling `ChildComponent` when a `ParentComponent` is attached directly (rather than
    building the relationship from the child's side) added a *default-constructed*
    `ChildComponent{}` - parent left null - instead of one actually pointing at the parent.
  - Destroying a parent entity iterated its `ParentComponent::children` **by reference** while
    destroying each child - and destroying a child re-enters `RelationshipSystem` for its own
    `ChildComponent` removal, which (still reachable, since the parent's `ParentComponent` isn't
    erased until after its own on_destroy handler returns) reached back into that *same* live
    vector and erased from it, invalidating the outer loop's iterators mid-iteration. This
    reliably corrupted the heap (`malloc(): unaligned tcache chunk detected`) the moment a
    parent had any children at all. The fix snapshots the children into a local vector up front
    and clears each child's back-reference *before* destroying it, so the reciprocal cleanup
    finds an already-invalid parent and skips touching the (still mid-destruction) `ParentComponent`
    entirely - verified crash-free under AddressSanitizer + UBSan, not just "didn't crash this run".
- **What's deliberately not here yet**: a component registry tying component types to
  Reflectable metadata (names, generic field access for an inspector/serializer) - the
  old critique flagged the total absence of this, and `Reflectable`/`for_each_field` already
  provide everything it'd need. `Components` below now gives it something real to validate
  against, but building the registry itself is still a separate, later step. Scene
  (de)serialization is the natural following step after that exists.
- **`AbstractSystem` gained a third cadence pair, `on_attach()`/`on_detach()`, plus `on_event(const
  Event&)`** - added for `Application` (see below), which needed to reinstate the old engine's
  attach/detach-per-scene lifecycle (used there for starting/stopping music) and event dispatch
  (the old `Scene::handle_event()`) without reintroducing either as a `Scene`-level virtual. Same
  shape as `update`/`render`: default no-op, opt-in per system. `on_event` is the one exception -
  it returns `bool` (`true` = consumed, stop propagating) instead of `void`, so e.g. a future
  `ImguAbstractSystem` can claim whatever ImGui's own `WantCaptureMouse`/`WantCaptureKeyboard` wants and
  keep it from reaching anything registered after it - directly restoring the old engine's ImGui
  input-gating behavior (previously hardcoded into `Keyboard`/`Mouse` themselves, stripped out
  earlier in this port per `architecture.md`), but as an ordinary, opt-in system instead.
  `SystemRegistry` gained matching `attach_all()`/`detach_all()`/`bool dispatch_event(const
  Event&)` (registration order, `dispatch_event` stopping at the first system that returns
  `true`), and `Scene` gained thin, non-virtual `attach()`/`detach()`/`dispatch_event()` forwards
  - same relationship `update()`/`render()` already have to `SystemRegistry::update_all()`/
  `render_all()`. Backward-compatible: existing systems that don't override the new hooks are
  unaffected. `Event`'s own `EventCallback` typedef (declared, never previously used anywhere)
  was updated to return `bool` for the same reason, rather than declaring a second, parallel
  callback type.

### Application
`draft/core/application.hpp`/`.cpp` (new `draft/core/` - `Scene`/`Registry` already moved to
`draft/ecs/`, but this isn't an ECS concept, it's the thing that *holds and drives* a `Scene`).
Runtime had no main-loop owner until now - every piece existed (`RenderWindow`, `Keyboard`/
`Mouse`, `Scene`/`SystemRegistry`, `Renderer`/`DefaultRenderer`) but nothing tied them together;
the editor example wired a loop by hand. `engine/include/draft/core/application.hpp`/`.cpp` is
the old engine's reference - most of its shape carries forward directly (owns window/keyboard/
mouse, a swappable `Renderer`, a swappable, non-owning `Scene*`, a fixed-timestep accumulator,
GLFW-callback-to-`Event` translation), with a few deliberate differences:

- **No `Scene::handle_event()`/`on_attach()`/`on_detach()` to call into** - the ECS redesign
  stripped every virtual off `Scene`. `Application` still translates raw GLFW callbacks into
  `Event`, same as before, but dispatches through the new `AbstractSystem`/`SystemRegistry`/`Scene`
  hooks described above instead: a private `dispatch(const Event&)` helper calls
  `eventCallback(event)` first (a new public `EventCallback eventCallback` member, for
  embedder-level concerns not tied to any particular `Scene` - an editor's own global shortcuts,
  say), and only if that's unset or returns `false` goes on to call
  `activeScene->dispatch_event(event)`. Registering a system on a scene (the already-existing
  `scene.get_systems().add<T>(...)`) is what "subscribes" it to both events and attach/detach -
  no separate listener-registration mechanism needed. `scene_change()` (driven by
  `set_scene()`) calls the outgoing scene's `detach()` and the incoming scene's `attach()`,
  restoring the old per-scene music-start/stop use case, just routed through whichever systems
  are actually registered rather than a single scene-wide virtual.
- **`run()`/`step()` split - the one deliberate API addition.** The old `Application` only
  exposed `run()`, an unconditional loop with no way to drive a single frame manually (and no way
  to test it - the old engine never had an `application.test.cpp` either, for the same reason).
  `run()` is now just `while(step());` - `step()` does exactly one iteration (apply pending
  resize/renderer/scene swaps, poll window events, run the fixed-timestep accumulator, render one
  frame, return whether the window is still open), directly testable against a real hidden
  `RenderWindow`-backed `Application` instance without fighting an infinite loop.
- **Dropped the old `frame()`'s second, unconditional `renderer->resize(window.get_frame_size())`
  call**, issued every single frame regardless of whether a resize actually happened - redundant
  with (and wasteful alongside) the `pendingResize`-gated resize already applied once at the top
  of the loop when the size actually changes. Reads like leftover/dead code from an earlier
  iteration of the old engine, not an intentional every-frame behavior; not carried forward.
  Regression-tested with a call-counting `Renderer` subclass confirming `resize()` fires exactly
  once per real size change, not on every subsequent `step()`.
- **ImGui/RmlUi/Console/Stats/Tracy are not ported.** `architecture.md` says Runtime has no
  editor-specific functionality (already why `Keyboard`/`Mouse` had their ImGui gating stripped
  earlier in this port), and Tracy is a cross-cutting exclusion from the whole Rendering roadmap.
  None of it belongs in this class - an `ImguAbstractSystem` (if/when ImGui integration happens) would
  be an ordinary `AbstractSystem` using the `on_event()` gating described above, not something baked
  into the loop driver itself.
- `editor/src/main.cpp` now uses `Application` in place of its hand-rolled loop, as the real
  end-to-end smoke test - the square/text-box demo content and the `ParticleEmitterSystem`
  showcase both became ordinary `AbstractSystem`s submitting into `Application::get_renderer()`'s shared
  `batch`/`shape` collections via `render(Time dt)`, registered on a `Scene` the `Application`
  holds and drives.

### Components
`draft/components/` - concrete data an entity can hold, as opposed to `draft/ecs/`'s mechanics.
Ported so far, marked `DRAFT_REFLECTABLE` wherever the fields are plain values (costs nothing
now, sets up whatever eventually consumes that metadata - a component registry, a generic
inspector, scene serialization - none of which exists yet):

- `TagComponent` - a `std::string` wrapper, unchanged from `engine/`.
- `TransformComponent` - position + rotation, plus `get_transform()`/`get_matrix()` building a
  3x3/4x4 matrix via `Math` (`Math::translate`/`Math::rotate`) - the first real Runtime consumer
  of Common's `Math` module. The old version's `force_sync(Entity)` method, which reached into
  `RigidBodyComponent`/`NativeBodyComponent`, isn't ported - Physics doesn't exist in Runtime
  yet. It comes back once `RigidBodyComponent` does.
- `TorqueComponent`/`ForceComponent`/`ImpulseComponent` and their `Continuous*` variants
  (`rigid_body_component.hpp`) - one-shot vs. every-physics-step torque/force/impulse *requests*.
  In the old engine, `PhysicsSystem` only ever queried these alongside `NativeBodyComponent`
  (which holds a live box2d body pointer) - applying a one-shot one and removing it, or applying
  a continuous one every step while `enabled`. Ported here as pure request data with no box2d
  coupling of their own; they're inert until a PhysicsSystem exists to read and act on them.
  `RigidBodyComponent`/`NativeBodyComponent` themselves aren't ported - both hold a live
  `RigidBody*`.
- `ConstrainedComponent` (`joint_component.hpp`) - the entities this entity is joint-constrained
  to. The old engine's `NativeJointComponent`/`JointComponent<T>`/per-joint-type components
  (`DistanceJointComponent`, `RevoluteJointComponent`, ...) aren't ported - all hold a live box2d
  `Joint*`. Not `DRAFT_REFLECTABLE`, matching `ChildComponent`/`ParentComponent` (`ecs/
  relationship_components.hpp`): it holds `Entity`s, not the kind of plain value a generic
  inspector/serializer can walk. Unlike everything else in this section, this one needs no
  not-yet-built subsystem at all to actually use - `Entity`/`Scene` already work today.

Everything still not ported here (`SpriteComponent`, `RigidBodyComponent`, `ColliderComponent`,
...) depends on a Runtime subsystem that doesn't exist yet (Rendering, Physics, Audio, Input are
all still placeholders).

### Input
`draft/input/` now has `Event`/`Action` (unchanged from `engine/`) plus `Keyboard` and `Mouse`.

- **`Event`/`Action`** - `Event` is a plain tagged union (`EventType` +
  `SizeEvent`/`KeyEvent`/`TextEvent`/`MouseMoveEvent`/`MouseButtonEvent`/`MouseWheelScrollEvent`)
  and `Action` is a 3-value enum (`PRESS`/`RELEASE`/`HOLD`). Neither is actually consumed by
  `Keyboard`/`Mouse` below - in the old engine these were produced by `Application` (translating
  raw GLFW callback ints before dispatching), not by `Keyboard`/`Mouse` themselves, which have
  their own raw-int callback surface (`KeyCallback`, `MouseButtonCallback`, ...). `Application`
  (or whatever replaces it) is still unported, so `Event`/`Action` remain a vocabulary without a
  producer yet. Not marked `DRAFT_REFLECTABLE`: transient per-frame messages, not persistent
  state.
- **`Keyboard`/`Mouse`** (`keyboard.hpp`/`.cpp`, `mouse.hpp`/`.cpp`) - install GLFW callbacks on a
  `RenderWindow` via the same `glfwGetWindowUserPointer` -> `Window*` -> `->m_keyboard`/`->m_mouse`
  dispatch pattern `Window` already uses for its own resize/focus/close callbacks (the
  `Keyboard*`/`Mouse*` members and `friend` declarations were added to `Window` back when it was
  first ported, sitting unused until now). Structurally unchanged from the old engine - same
  `Key`/`Modifier`/`Button` enums, same immediate (`is_pressed`) vs. edge-triggered
  (`is_just_pressed`) query surface, same move-only ownership.
  - **Dropped all ImGui coupling.** The old `Keyboard::is_pressed()` gated real GLFW state behind
    `!ImGuiIO::WantCaptureKeyboard`; every `Mouse` callback early-returned on
    `ImGuiIO::WantCaptureMouse`, and `button_pressed` additionally forwarded every click into
    ImGui's own input queue. This isn't a dead include like `RenderWindow`'s old
    `imgui_engine.hpp`/`rml_engine.hpp` - it's real behavior - but it's exactly what
    `refactor/architecture.md` rules out ("The Runtime contains no editor-specific
    functionality"). `Keyboard`/`Mouse` now report raw GLFW state unconditionally; whatever
    eventually consumes input in the Editor is responsible for its own UI-capture suppression.
  - **`Mouse::get_normalized_position()`** no longer calls a `RenderWindow`-taking
    `Math::normalize_coordinates()` overload - that overload (left out of Common when `Math` was
    ported, since `RenderWindow` didn't exist yet) turned out to not actually delegate to
    Common's `Rect`-taking overload despite looking like it does (a `UIntRect` and a `Vector2d`
    don't share a template parameter, so it couldn't). It's the same handful of lines inlined
    directly instead, calling nothing new - no Math surface needed anywhere for this.
  - **Found and fixed two real bugs, both about stale pointers after a move**: neither the old
    engine's `Keyboard`/`Mouse` move constructor/assignment nor `Window`'s own updated the
    window's `m_keyboard`/`m_mouse` (or, symmetrically, `Keyboard`/`Mouse`'s own `m_window`) to
    point at the *new* object - so moving either one left the window dispatching key/mouse
    events into a moved-from, empty object instead of the one actually holding the callbacks.
    Fixed by re-pointing both sides on every move (`Keyboard`/`Mouse` update
    `m_window->m_keyboard`/`m_mouse`; `Window`'s own move ctor/assignment update
    `m_keyboard`/`m_mouse`'s `m_window` back-pointer, `static_cast<RenderWindow*>(this)`).
    `Window::~Window()` also now genuinely notifies a live `Keyboard`/`Mouse` that their window
    died (previously a placeholder comment, since neither class existed yet).
  - **Verified against a real hidden window** (construction/destruction correctly freeing the
    window's registration slot for reuse, deterministic "nothing is pressed/hovered" state, move
    semantics). `Mouse::set_position()`/`get_position()` can't be verified end-to-end here -
    confirmed via a raw, `Mouse`-bypassing GLFW check that this sandbox's window manager never
    grants focus to a window it didn't create interactively
    (`glfwGetWindowAttrib(..., GLFW_FOCUSED)` reads 0 even for a real visible window), and
    `glfwSetCursorPos` only takes effect on a focused window - the same category of environment
    limitation as `Window::set_icon` under Wayland. `get_normalized_position()`'s formula is
    still verified against whatever position the sandbox actually reports, which still catches a
    broken formula (wrong axis, missing y-flip, wrong window size).

### Physics
`draft/phys/` has every piece of `engine/include/draft/phys/` that's pure value data with no
live box2d handle and no logic of its own to reimplement, all unchanged from `engine/` and
marked `DRAFT_REFLECTABLE` unless noted:

- `PhysMask` (`filter.hpp`) - collision filtering (`categoryBits`/`maskBits`/`groupIndex`): two
  fixtures collide only if each one's category is in the other's mask, unless a shared nonzero
  group overrides that.
- `MassData` (`mass_data.hpp`) - mass, center of mass, rotational inertia; the shape of value
  `b2Body::GetMassData()` returns.
- `RaycastProps`/`RaycastResult` (`raycast_props.hpp`) - raycast query parameters and hit result,
  alongside `ShapecastProps` and the `RaycastCallback` function-type alias (both landed in Phase
  2 once `Shape&`/`Fixture*` existed to reference - `ShapecastProps` has zero callers in the old
  engine too, ported anyway rather than pruned as dead).
- `FixtureDef` (`fixture_def.hpp`) - shape + material/collision properties blueprint. `shape` is
  only ever a pointer here, so this compiles with just a forward-declared `Shape` - Runtime has
  nothing concrete to point it at yet, but the descriptor itself needs nothing from Physics. Not
  `DRAFT_REFLECTABLE`: `shape` is a raw pointer.
- `GenericJointData` and the ten concrete `*JointData` structs (`joint_data.hpp`) -
  `DistanceJointData`, `FrictionJointData`, `GearJointData`, `MotorJointData`, `MouseJointData`,
  `PrismaticJointData`, `PulleyJointData`, `RevoluteJointData`, `WeldJointData`,
  `WheelJointData`. These are the value-data half of the old engine's
  `JointComponent<T>::delta` - split out from `JointDef`/the `*JointDef` constructors (need
  `RigidBody*` and `Joint::Type`) and from `Joint`/the `*Joint` handle classes (hold a live
  b2Joint). `GearJointData::joint1`/`joint2` are raw pointers, so only `ratio` is reflected there.

**Physics Roadmap Phase 1 (`refactor/physics_roadmap.md`) is now done** - box2d v2.4.1 is a real
`draft_runtime` dependency (`FetchContent`-pinned, linked `PRIVATE`, matching GLFW/GLAD/
Freetype/tinygltf's discipline) and the `Shape` hierarchy has landed:

- **`Shape`** (`phys/shapes/shape.hpp`) - zero box2d coupling, ported unchanged: `ShapeType`
  enum, `isConvex`/`isSensor`/`friction`/`restitution`/`density`, pure-virtual `clone()`/
  `contains()`.
- **`CircleShape`/`EdgeShape`/`PolygonShape`/`ChainShape`** (`phys/shapes/`) - public headers are
  straight ports, no box2d includes. `vector_to_b2`/`b2_to_vector` (Vector2 only - the old
  engine's Vector3 sibling was confirmed dead: zero callers, and didn't even compile if
  instantiated) landed as a private `phys/vector2_p.hpp`, matching the checklist's plan to home
  these in Physics rather than Common. `conversions_p.hpp`/`conversions.cpp` also land here for
  the first time, carrying only the `shape_to_b2()` slice Phase 1 needs - later phases add more
  overloads to the same two files rather than porting them atomically.
- **Three real bugs found and fixed**:
  - `ChainShape::clone()` copied every field except `m_prev`/`m_next` - every sibling shape's
    `clone()` copies 100% of its state, Chain's didn't, silently resetting a cloned `CHAIN`-type
    shape's open-end smoothing to garbage. Fixed by copying both fields; regression-tested by
    reverting locally and confirming the new test failed first (it did - to uninitialized stack
    garbage, not even a clean `{0,0}`).
  - `PolygonShape` had no bound check against box2d's hard `[3, b2_maxPolygonVertices]` (8)
    vertex limit before `shape_to_b2()` → `b2PolygonShape::Set()` wrote into box2d's fixed
    `b2Vec2[8]` internal array - guarded only by an `assert` (compiled out under `NDEBUG`), a
    real buffer overrun in release builds. Fixed by validating the count and throwing
    `std::runtime_error` first (setup-time, matching the `exit(0)`→exception rule already used
    throughout Rendering); also replaced the old VLA (`b2Vec2 physVerts[count];` - a GCC/Clang
    extension, not standard, MSVC-incompatible) with a bounded `std::array<b2Vec2, 8>` now that
    `count` is validated. Confirmed as a real crash, not just theoretical UB: reverting the fix
    and running the "too many vertices" regression test aborted immediately under this sandbox's
    hardened libstdc++ (`std::array::operator[]`'s bounds assertion firing on the raw write) -
    the exact same "hardened build catches it" confirmation style used for Rendering's `Renderer::
    set_state()` scissor bug.
  - `EdgeShape`/`ChainShape::contains()` used to round-trip through `shape_to_b2()` +
    `b2*Shape::TestPoint()` for a result that's always `false` - box2d's own `TestPoint()` for
    both shape types is hardcoded to `return false` unconditionally (an infinitely-thin edge/chain
    has no interior), never reading either argument. Not a behavior bug (the old engine's
    `contains()` for these two shapes always returned `false` too), but reimplemented as a direct
    `return false;` rather than constructing a box2d shape just to reach a constant. `CircleShape::
    contains()` got the same "skip the round trip" treatment, but is a real port of live logic:
    box2d's `b2CircleShape::TestPoint()` is a genuine 2-line `distance(point, center) <= radius`
    check, reimplemented directly via `Math::distance2` instead of via box2d.
  `PolygonShape::contains()` is the one shape that still routes through `shape_to_b2()` +
  `TestPoint()` - it genuinely needs box2d's convex-hull/normal computation, since `PolygonShape`
  never validates or computes a hull client-side; reimplementing that would be new logic, not a
  port.

**Physics Roadmap Phase 2 is also done.** While surveying the old engine to start it, a fresh
`EnterPlanMode` pass found the phase boundary didn't hold up: `RigidBody`'s constructor is
private, friend-constructed only by `World` (`friend class World;`), and every single old-engine
test for `RigidBody`/`Fixture`/`Collider` constructs a real `World` first - confirmed by reading
all of them, not assumed. `World` itself only depends on `Joint`/`JointDef` (Phase 3) for
`create_joint`/`destroy_joint`, and on `PhysicsDebugRender` (Phase 6) for debug rendering -
everything else it does (body create/destroy, gravity, origin-shift, `step()`, `raycast()`,
`test_point()`) has zero dependency on either. So Phase 2 pulled forward exactly that Joint-free,
debug-render-free slice of `World` - otherwise nothing in this phase could compile+link+test
standalone (a static-library test binary pulls in whole translation units; `rigid_body.cpp`
referencing `World::destroy_body`/`get_shift_offset` with no `World` at all is an
unresolved-symbol link failure the instant any `RigidBody` test runs). `refactor/
physics_roadmap.md`'s Phase 4 (originally "World") is now folded into Phase 2 (this core) +
Phase 3 (joint wiring, which extends the same `world.hpp`/`world.cpp` this phase created).

- **`phys/body_type.hpp`** - `RigidBody::BodyType` hoisted out into a free `enum class BodyType`
  (`STATIC`/`KINEMATIC`/`DYNAMIC`) - `BodyDef`'s only dependency on `RigidBody` was naming this
  one nested enum, and a fresh port has no back-compat reason to keep it nested. `BodyDef`
  (`phys/body_def.hpp`) is a straight port of the old struct against this hoisted enum.
- **`Fixture`** (`phys/fixture.hpp` + `src/draft/phys/fixture.cpp`) - pImpl'd `b2Fixture*`
  wrapper, friend-constructed by `RigidBody`; straight port, no bugs found.
- **`RigidBody`** (`phys/rigid_body.hpp` + `src/draft/phys/rigid_body.cpp`) - pImpl'd `b2Body*`
  wrapper, friend-constructed by `World`, owns its `Fixture`s via `vector<unique_ptr<Fixture>>` +
  a `b2Fixture*->Fixture*` lookup map. Gained a new non-owning `vector<Collider*>
  attachedColliders` member for the dangling-pointer fix below. Kept its own local
  `convert_to_b2`/`convert_to_draft` `BodyType`<->`b2BodyType` pair separate from
  `conversions_p.hpp`'s functionally-identical `bodytype_to_b2` rather than consolidating them -
  per instruction, old-engine duplication isn't treated as a defect to clean up during this port.
- **`Collider`** (`phys/collider.hpp` + `src/draft/phys/collider.cpp`) - owns `Shape`s, attaches
  to a `RigidBody` by creating fixtures. `Collider(const JSON&)`'s hand-rolled mesh-loading
  constructor stays deferred (waits on Serialization, not started - same reasoning as
  `AssetManager`'s concrete loaders waiting on Rendering/Audio types before they existed).
- **`World` (core)** (`phys/world.hpp` + `src/draft/phys/world.cpp`) - pImpl'd `b2World` wrapper:
  `create_rigid_body`/`get_body`/`destroy_body`/`get_body_count`, `get_gravity`/`set_gravity`/
  `shift_origin`/`get_shift_offset`, `step`, `raycast`/`test_point` (via `B2RaycastProxy`,
  `src/draft/phys/b2_raycast_proxy_p.hpp`/`.cpp` - a private `b2RayCastCallback` shim, straight
  port). `create_joint`/`destroy_joint` (Phase 3) and `set_debug_renderer`/`debug_draw`/the
  `defaultShader` static (Phase 6) aren't here yet. `set_destruction_listener` is dropped
  entirely - confirmed dead (declared in the old header, never defined anywhere, would fail to
  link if called).
- **`conversions_p.hpp`/`conversions.cpp`** grow their next slice: `bodytype_to_b2`,
  `bodydef_to_b2`, `filter_to_b2`, `raycast_to_b2`/`b2_to_raycast_result`, and `transform_to_b2` -
  the last of these ported even though it has zero callers anywhere in the old engine, per
  instruction not to prune old-engine code for looking unused.
- **Two more real bugs found and fixed**:
  - **`Collider` dangled in both directions relative to its attached `RigidBody`** - a real
    design change, not a one-line patch. *Collider destroyed first*: the old `Collider` had no
    user-declared destructor, so destroying an attached `Collider` never destroyed the
    corresponding box2d fixtures - they leaked in the live `b2Body` forever, and every
    `Fixture::shapePtr` pointing at the `Collider`'s now-destroyed `Shape`s dangled. *RigidBody
    destroyed first*: nothing invalidated a `Collider` still pointing at a destroyed `RigidBody`
    - `collider.set_position()` → `update_collider()` → `attach()` dereferencing the dead body,
    a real UAF. Fixed by giving `RigidBody` the `attachedColliders` list above, making
    `RigidBody`/`Collider` mutual friends for this bookkeeping only: `Collider::attach()`
    registers itself, `Collider::detach()` unregisters itself (in addition to destroying its
    fixtures via the body, as before); a new `Collider::~Collider()` calls `detach()` if still
    attached (fixes the first direction); `RigidBody`'s destructor snapshots + clears
    `attachedColliders` then calls a new `Collider::invalidate()` (no box2d calls) on each,
    setting `rigidBodyPtr = nullptr` and clearing the local `fixtures` list (fixes the second
    direction) - the same "snapshot + clear back-reference before destroying" shape already
    proven correct for `RelationshipSystem::deconstruct_parent_func`'s parent/child destruction
    fix. Both directions regression-tested: reverting the destructor fix and destroying an
    attached `Collider` leaves the body's fixture count non-zero (confirmed); reverting the
    `RigidBody`-destructor fix leaves `collider.is_attached()` still `true` after the body is
    destroyed (confirmed - the assertion fails before the test can reach the actual UAF in a
    following `set_position()` call).
  - **`RigidBody::get_fixture(void*) const` *and* `World::get_body(void*) const` both used
    `unordered_map::operator[]`** on their lookup map, which inserts a bogus null entry on every
    miss despite both methods being declared `const` (legal - `unique_ptr<Impl>`'s constness
    doesn't propagate to the pointee - but not what the `const` signature implies). The roadmap
    flagged the first; the second is the byte-identical bug shape, found via the same
    structural-sibling comparison already used elsewhere in this port (e.g. Rendering's
    cull-face fix). Both swapped to `.find()`. Not independently regression-testable from the
    public API - `operator[]` and `.find()` both return `nullptr` on a miss (`Fixture*`/
    `RigidBody*` default-construct to `nullptr` either way) - fixed anyway for real
    const-correctness and to stop the map growing forever on repeated misses; described modestly
    here rather than oversold, same as `ParticleSystem`'s minor pool-recycling fix.

**Physics Roadmap Phase 3 is also done** - the `Joint` hierarchy and `World::create_joint`/
`destroy_joint`. No phase-boundary surprises this time (unlike Phase 2): `Joint`'s
friend-constructor is `World` (already ported), and `*JointDef`'s only dependency (`RigidBody*`,
`Joint::Type`) was already ported too.

- **`Joint`** base (`phys/joint.hpp` + `src/draft/phys/joint.cpp`) - pImpl'd `b2Joint*` wrapper,
  friend-constructed by `World`. Its `protected get_joint_ptr()` is friended by
  `jointdef_to_b2(const GearJointDef&)`, which needs to reach into `joint1`/`joint2`'s live
  handles - same shape as the old header's `friend b2GearJointDef jointdef_to_b2(const
  GearJointDef& def);`.
- **11 concrete subclasses** (`DistanceJoint`...`WheelJoint`) - thin wrappers over their
  `b2*Joint` counterpart, straight ports, no bugs found. `Type::ROPE` stays a vestigial enum
  value with no subclass (box2d 2.4 dropped `b2RopeJoint` in favor of `DistanceJoint`'s min/max
  length).
- **10 `*JointDef` structs** (`phys/joint_def.hpp`) - each pairs `JointDef` (base: `type`/
  `bodyA`/`bodyB`) with its already-ported `*JointData` via the old engine's diamond-inheritance
  shape (`*JointDef : JointDef, *JointData`).
- **`RigidBody`** gained the `friend void jointdef_base_to_b2(const JointDef&, b2JointDef&);`
  declaration deferred from Phase 2 (since `JointDef` didn't exist yet), so that free function
  can reach `RigidBody::get_body_ptr()` for `bodyA`/`bodyB`.
- **`conversions_p.hpp`/`conversions.cpp`** grew their final slice: `jointtype_to_b2`,
  `jointdef_base_to_b2`, and the 10 `jointdef_to_b2(const *JointDef&)` overloads.
- **`World`** (editing the existing `world.hpp`/`world.cpp` from Phase 2, not new files) gained
  `create_joint(const JointDef&)`/`destroy_joint(Joint*)` and a `vector<unique_ptr<Joint>> joints`
  member.
- **Two more real bugs found and fixed**:
  - **`GearJointDef`'s conversion unconditionally dereferenced `def.joint1->get_joint_ptr()`/
    `joint2->...`** with no null check, even though both default to `nullptr` - a real segfault
    for any `GearJointDef` built without setting them, not hypothetical. Fixed by throwing
    `std::invalid_argument` first (setup-time, matching the `exit(0)`→exception rule already
    used throughout Rendering and Phase 1).
  - **`World::create_joint`'s `switch` called `exit(0)` on `Type::ROPE`/`UNKNOWN`/`default`** -
    replaced with a thrown exception, same rule.
  - **Known limitation, documented not fixed**: a live `GearJoint`'s `joint1`/`joint2` become
    dangling if either constituent joint is destroyed independently - nothing tracks
    back-references to catch this (box2d itself has the same limitation internally). A real fix
    needs an observer/registry mechanism disproportionate to this phase's scope - documented
    instead of "fixed", the same way `Camera::project()`'s missing perspective divide was.

**Physics Roadmap Phase 5 is also done** - the ECS integration: the missing physics components
and reworking `PhysicsSystem` into an ordinary `AbstractSystem`. Unlike Phases 1-3, this wasn't new
box2d-handle plumbing - it's wiring the already-ported physics types into the ECS via ~46
`on_construct`/`on_destroy`/`on_update` signal connections, the exact pattern already proven in
`RelationshipSystem` (`draft/ecs/relationship_system.cpp`). The roadmap's own "first thing to
verify" concern - whether engine-refactor's `Registry` exposes the same signal API the old one
relied on - turned out to be a non-issue: `Registry` (`draft/ecs/registry.hpp`) is a bare
`entt::registry` alias, so `on_construct`/`on_destroy`/`on_update` are just EnTT's own API.

- **`rigid_body_component.hpp`** gained `NativeBodyComponent` (raw `RigidBody*` + delta-tracking
  fields, against the hoisted `BodyType`) and `RigidBodyComponent` (pure data + a raw
  `RigidBody* m_nativeHandlePtr`) - straight ports. Neither is `DRAFT_REFLECTABLE` (both hold a
  raw pointer, same reasoning as `FixtureDef`).
- **New `collider_component.hpp`**: `ColliderComponent` (owns a `Collider`) - straight port.
- **`joint_component.hpp`** gained `NativeJointComponent<T>`, `JointComponent<T>`, the 10 concrete
  `*JointComponent` types (`DistanceJointComponent`...`WheelJointComponent`), the
  `DRAFT_ALL_JOINT_TYPES` macro + `JointComponentTypes` tuple, and `create_joint_entity()` -
  straight ports, none reflectable (raw `Joint*` member).
- **`transform_component.hpp`** gained back `force_sync(Entity)` (needs `NativeBodyComponent`,
  which now exists) - resolves the checklist's tracked follow-up.
- **New `draft/ecs/physics_system.hpp`/`.cpp`**: `PhysicsSystem : public AbstractSystem` (moved out of
  the old `draft/systems/`, same location as `AudioSystem`/`RenderSystem`) - a real design change
  from the old engine's `Scene`-owned special case with its own `World world{{0.f, 0.f}};` public
  member. Constructor is `PhysicsSystem(Scene& sceneRef, World& worldRef)`: `Scene&` for the same
  reason `RelationshipSystem` needs it (constructing `Entity` handles from raw `entt::entity`
  inside signal callbacks, e.g. for `ConstrainedComponent::constraints`), `World&` injected rather
  than owned (matching `RenderSystem` taking `Renderer&` externally). The old `physicsTimestep`/
  `Application*` override is dropped entirely - `dt` already arrives pre-fixed-stepped from
  `SystemRegistry::update_all(dt)` (`AbstractSystem::update(Time dt)`), making it redundant.
  `update(Time dt)` is now just `world.step(dt, ...); handle_joints(); handle_forces();
  handle_bodies();` - the old per-tick body minus the timestep-swap dance. All private helpers
  (`construct_body_func`, `construct_native_body_func`, `construct_collider_func`,
  `construct_joint_func<T>`/`construct_native_joint_func<T>` - the `dynamic_cast`-dispatch
  `*JointDef` builder - the `deconstruct_*` mirrors, `handle_bodies()`/`handle_forces()`/
  `handle_joints()`) are straight ports of the old logic. Tracy stripped, same as everywhere else.
- **One more real bug found and fixed**: **`RigidBodyComponent::allowSleep` was never synced
  after construction.** `NativeBodyComponent::deltaAllowSleep` exists for exactly this purpose
  (mirroring `deltaType`/`deltaAwake`/`deltaFixedRotation`/etc., every one of which *is* synced in
  `handle_bodies()`), but `handle_bodies()` never read or wrote it - confirmed by reading the
  function in full, it's the one field with a delta-tracking slot wired to nothing. Changing
  `RigidBodyComponent::allowSleep` at runtime silently had zero effect, unlike every sibling
  field. Root cause: `RigidBody` (Phase 2) never got an `is_sleep_allowed()` getter to read the
  state back (it had `set_sleep_allowed()` but no getter - box2d's `b2Body::IsSleepingAllowed()`
  exists and was simply never exposed). Fixed by adding `RigidBody::is_sleep_allowed() const`
  (`phys/rigid_body.hpp`/`.cpp`, same one-line shape as its `is_awake()`/`is_bullet()` siblings),
  then wiring the missing sync line into `handle_bodies()`.
- **Noted, not fixed**: `handle_bodies()`'s continuous-field sync (`linearVelocity`/
  `angularVelocity`/`linearDamping`/`angularDamping`/`gravityScale`) guards each resync with
  `if(Math::abs(delta) >= 0.f)` - since `abs(x) >= 0` is always true, this unconditionally
  re-issues the corresponding `RigidBody::set_*` call every tick for every body, even when
  nothing changed (the delta is 0, so it's a harmless no-op - `set(get() + 0)` - just redundant).
  Not a correctness bug (output is identical either way); ported as-is rather than "fixing" a
  tautological condition that doesn't change behavior.
- **Tests**: since the old engine has no dedicated test file for `PhysicsSystem` or these
  components, `engine-refactor/runtime/tests/draft/{components/{rigid_body_component,
  collider_component,joint_component},ecs/physics_system}.test.cpp` are new coverage (not ports),
  exercising the actual signal-driven lifecycle end-to-end through a real `Scene`+`World`: a
  `RigidBodyComponent` produces a valid `NativeBodyComponent`; a `ColliderComponent` attaches its
  fixtures once a body exists; destroying the entity cleans up both without dangling; a joint
  component pair between two body-having entities creates a live `Joint` and populates
  `ConstrainedComponent` on both ends (and destroying it un-populates them); a one-shot
  `TorqueComponent` is consumed after one `update()` while a `ContinuousForceComponent` isn't;
  and modifying a `TransformComponent` force-syncs the native body immediately via the
  `on_update` signal.

**Physics Roadmap Phase 6 is also done - `refactor/physics_roadmap.md` is now fully complete.**
The roadmap had flagged an open design question here: `PhysicsDebugRender` (a `b2Draw` subclass)
needed a `begin()`/`end()`-shaped interface that looked like it no longer existed on `Collection`/
`ShapeCollection`, since the old `Batch` class (the interface's only source) was dropped as dead
compat scaffolding during the Rendering port, with `PhysicsDebugRender` flagged as `Batch`'s one
remaining out-of-scope caller at the time. Surveying both sides resolved it cleanly:

- **Old** `engine/src/draft/rendering/phys_renderer_p.hpp`/`.cpp`: `PhysicsDebugRender` holds a
  `ShapeBatch batch` member (composition) and calls `batch.set_proj_matrix(m)`/`batch.begin()` in
  its own `begin()`, `batch.end()` in `render()`, and `batch.set_color()`/`draw_polygon()`/
  `draw_circle()`/`draw_line()` from the `b2Draw` overrides. Old `ShapeBatch` turned out to be
  nothing but `class ShapeBatch : public Batch, public ShapeCollection` - a thin multi-inheritance
  shim existing *only* to satisfy `Batch`'s pure-virtual `flush()`/`set_proj_matrix()`/
  `set_trans_matrix()` signatures; all the actual drawing was already old `ShapeCollection`'s.
- **Current** `ShapeCollection`/`Collection` (`draft/rendering/batching/{collection,
  shape_collection}.hpp`) **already has everything needed**: `Collection::set_proj_matrix(const
  Matrix4&)` (the old `begin()`'s matrix half) and `ShapeCollection::flush()` (the old `end()`,
  confirmed self-draining via its `while(!m_drawCommands.empty())` loop, so no extra per-frame
  reset is needed), plus `set_color()`/`set_render_type()`/`draw_polygon()`/`draw_circle()`/
  `draw_line()` under the same names. **No `Collection` changes were needed at all** - the
  roadmap's first option (add a minimal `begin()`/`end()` back) turned out to be unnecessary;
  this landed squarely as the second ("rewrite `PhysicsDebugRender` around `ShapeCollection::
  flush()`"), and it's a smaller rewrite than anticipated since the base class already had the
  matrix setter too.

- **`PhysicsDebugRender`** (new `src/draft/rendering/phys_renderer_p.hpp`/`.cpp`, same location
  as the old file - it's a renderer implementing `b2Draw`, Rendering's territory even though
  `World` is its only caller) holds a `ShapeCollection collection` member instead of `ShapeBatch`.
  `begin(const Matrix4&)` calls `collection.set_proj_matrix(m)`; `render()` calls `collection.
  flush()`. The 7 `b2Draw` overrides (`DrawPolygon`/`DrawSolidPolygon`/`DrawCircle`/
  `DrawSolidCircle`/`DrawSegment`/`DrawTransform`/`DrawPoint`) are straight ports, swapping
  `batch.` for `collection.` - identical method names/signatures throughout.
- **`World`** (editing the existing `phys/world.hpp`/`world.cpp`, no new files) gained
  `set_debug_renderer(Resource<Shader> shader = default_shader(), void* renderer = nullptr)`,
  `debug_draw(const Matrix4& m = Matrix4(1.f))`, and `Impl::physRenderer` (`unique_ptr<
  PhysicsDebugRender>`). The old `static StaticResource<Shader> defaultShader` member became a
  private `static Resource<Shader> default_shader()` (leaked-`new`, process-lifetime,
  context-bound), the exact pattern already established for `ShapeCollection`/`SpriteCollection`'s
  own default shaders - `StaticResource<T>` doesn't exist in engine-refactor at all. `world.hpp`
  gained `#include`s for `draft/rendering/shader.hpp` and `draft/asset/resource.hpp` - a real, new
  Physics→Rendering coupling, matching the old engine's own `World`, which had exactly this
  dependency.
- **One more real bug found and fixed**: **`World::debug_draw()` unconditionally dereferenced
  `physRenderer`** with no check that `set_debug_renderer()` was ever called - guarded with a
  debug-build `assert` (a per-frame call, so an assert-guarded contract rather than a thrown
  exception, matching how `Entity`'s accessors were hardened during the ECS redesign).
- Tracy stripped from the old `debug_draw()` (`ZoneScopedN`/`TracyGpuZone`), cross-cutting, same
  as every prior phase.
- **New testing wrinkle**: every prior Physics phase was headless (box2d needs no GL) - this is
  the first Physics test that needs a real GL context, since constructing a `ShapeCollection`
  issues real GL calls. `tests/draft/phys/world.test.cpp` gained a `WorldDebugRenderTest` fixture
  mirroring the hidden-`RenderWindow` pattern already proven in `tests/draft/rendering/batching/
  shape_collection.test.cpp` (`SetUpTestSuite`/`TearDownTestSuite`, a shared invisible window for
  the whole fixture) - one test confirms `debug_draw()` after `set_debug_renderer()` produces no
  GL error with a real body/fixture to draw, another confirms `debug_draw()` before ever calling
  `set_debug_renderer()` trips the new debug assert (`ASSERT_DEATH`, skipped in non-debug builds
  since the contract itself is debug-only).

This closes out `refactor/physics_roadmap.md` - all 6 phases are done.

### Audio
`draft/audio/` has `Listener`, `SoundBuffer`, `Sound`, and `Music`, plus `draft/components/
audio_components.hpp` (`SoundComponent`/`MusicStorage`/`MusicComponent`/`ListenerComponent`) and
`draft/ecs/audio_system.hpp` (`AudioSystem`).

**Kept SFML::Audio rather than moving to OpenAL directly.** The checklist originally expected
this module to land on raw OpenAL, matching how Time/Window/Input already dropped SFML in favor
of `std::chrono`/GLFW - but that's a substantially bigger change (device/context management, a
decoder for compressed formats, and - since the old `Music` streams rather than decoding up
front - a double-buffered streaming queue with its own background thread, all work SFML already
does internally). Given the choice, this pass keeps wrapping `sf::Listener`/`sf::SoundBuffer`/
`sf::Sound`/`sf::Music` exactly like `engine/` did - same pImpl shape, same public API - so SFML
remains a Runtime dependency (linked `PRIVATE`, same as GLAD/GLFW: nothing in these headers
exposes an `sf::` type) until a dedicated pass decides to replace it. `SFML_BUILD_AUDIO` is the
only SFML module built here; `SFML_BUILD_WINDOW`/`GRAPHICS`/`NETWORK` are off - Runtime has no use
for them.

- **`Listener`** - global 3D listener position/orientation/volume, cached locally and pushed to
  OpenAL via `apply()`. Unchanged from `engine/`.
- **`SoundBuffer`** - decoded PCM samples, loaded from a `FileHandle` or raw bytes, shared by any
  number of `Sound`s. `reload()` re-decodes from the backing `FileHandle` (a no-op for a buffer
  built from raw bytes, since there's nothing on disk/embedded to re-read).
- **`Sound`** - one playable instance bound to a `SoundBuffer`. **Found and fixed a real bug**:
  the old copy constructor and `operator=` copied the underlying `sf::Sound` (which itself
  correctly copies its buffer binding) but never copied `m_bufferPtr`, the pointer `get_buffer()`
  actually returns - so a copied `Sound` would report `get_buffer() == nullptr` even though it
  was genuinely bound to and playing from a buffer. Fixed by copying `m_bufferPtr` alongside the
  `sf::Sound` in both places.
- **`Music`** - a streamed track, decoded incrementally by SFML as it plays rather than up front
  like `SoundBuffer`. `load(const std::string&)`/the string constructor still hand a raw path
  straight to `sf::Music::openFromFile()`, carried over as-is from `engine/` - SFML reads chunks
  from that file on demand rather than holding the whole thing in memory, which is worth keeping
  for a known real disk path. Also has `load(const FileHandle&)`/a matching constructor now, so
  an embedded (cmrc-backed) track works the same way `SoundBuffer` already could: it reads the
  whole file up front via `handle.read_bytes()` and calls `sf::Music::openFromMemory()` instead -
  `FileHandle` has no partial/streaming read of its own, so this trades "never holds the whole
  file in memory" for "works with any `FileProvider`, not just disk." The raw bytes are kept
  alive in `Music::Impl` for as long as the `sf::Music` needs them, since `openFromMemory()`
  streams the decode directly from that buffer rather than copying it up front - `sf::SoundBuffer`
  doesn't need this since it decodes and copies everything into its own internal buffer inside
  `loadFromMemory()` itself.
- **`audio_components.hpp`** - `SoundComponent` (owns a `Sound`), `ListenerComponent` (owns a
  `Listener` + a `priority` used to pick the active listener when several exist), and
  `MusicComponent` (a non-owning `Music*`) paired with `MusicStorage` (the actual
  `unique_ptr<Music>` owner) - split the same way `engine/` split them, since `Music` isn't
  copyable and a component needs to be. Unchanged from `engine/`.
- **`AudioSystem`** (`draft/ecs/audio_system.hpp`) - every frame (`render(dt)`, not the fixed-step
  `update(dt)` - audio panning should track the latest transform every frame, the same reasoning
  as rendering, not a physics-rate concern), syncs every `ListenerComponent`/`SoundComponent`/
  `MusicComponent`'s position from its entity's `TransformComponent` and applies whichever
  listener(s) were found. **The old engine's doppler pitch shift is deliberately not ported** -
  it read a source/listener's velocity off `NativeBodyComponent` (a live `RigidBody*`), which
  doesn't exist in Runtime until Physics lands (see "Physics" above). `dopplerSensitivity` is
  kept as a public field so callers don't need to touch this again once it comes back; re-adding
  it is a small, self-contained follow-up once `RigidBodyComponent` exists.
- **Verified against a real audio device** (this sandbox has ALSA/PulseAudio and a working
  `libopenal`) rather than just compiling: buffer decode round-trips through both raw bytes and a
  real `FileHandle`-backed WAV file, `Sound`/`Music` property getters/setters, and `AudioSystem`
  syncing positions through a real `Scene`. One environmental quirk observed, not a bug: SFML's
  global `Listener` calls only reach a live OpenAL context while at least one `Sound`/`Music`
  object exists anywhere in the process (SFML lazily tears down its shared audio device once the
  last one is destroyed) - a test that calls `Listener::apply()` with no live sound source prints
  benign `AL_INVALID_OPERATION` diagnostics to stderr but still passes, since `Listener` itself
  keeps tracking state correctly regardless. The `AudioSystem` tests keep one `Sound` alive for
  exactly this reason.

## Known non-obvious fixes made during the port
- `DiskFileProvider::last_modified()` had to switch from taking `fs::file_time_type`'s
  `time_since_epoch()` directly to going through `std::chrono::file_clock::to_sys()` first -
  the former produced nonsense values (off by tens of thousands of years) because
  `file_time_type`'s clock has an implementation-defined epoch that doesn't line up with
  `system_clock` on at least one toolchain tested (libc++). A test now checks the value
  against real wall-clock time specifically because a weaker "is it non-negative" check would
  not have caught this.
- `json.cpp`'s `encode_json`/`decode_json` didn't match their own header declarations
  (the header declared a reference-taking overload that was never defined) and `decode_json`
  was an unconditional `nullptr` stub; both are now real, minimal, working forwarders onto
  `JSONInterface`.
- `Sound`'s copy constructor/`operator=` (see "Audio" above) copied the underlying `sf::Sound`
  but not `m_bufferPtr`, so a copied `Sound` silently reported no bound buffer via `get_buffer()`
  despite actually playing from one.
- Rendering Roadmap Phase 3/4's "lazily-constructed, process-lifetime" fallback statics
  (`Material3D`'s 3 debug textures, `ShapeCollection::default_shader()`) were first written as
  plain function-local `static T`s, reasoning this was equivalent to the old engine's
  `StaticResource<T>` ("used ONLY for debug and other bullshit static defaulted resources...
  anything else will [leak]" - its own doc comment). It isn't: `StaticResource<T>`'s destructor
  is empty and never deletes its wrapped pointer, so the object is never destructed at all; a
  plain `static T` *does* run its destructor at process exit, after every `RenderWindow`/GL
  context in the process has already been torn down - calling `glDeleteTextures`/`glDeleteProgram`
  with no current context, which segfaults in the driver. Confirmed as a real crash (not
  hypothetical) via `coredumpctl gdb` on the test binary - a SIGSEGV inside `Texture::cleanup()`,
  called from a static destructor during `exit()`, surfacing as a genuine OS-level crash
  notification even though every individual test had already reported "PASSED" (the crash happens
  during global teardown, after `RUN_ALL_TESTS()` returns). Fixed by actually matching
  `StaticResource<T>`'s real behavior: a leaked `static T* = new T(...)`, never `delete`d, so
  there's no destructor to run at exit at all.
