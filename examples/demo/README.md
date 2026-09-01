# WebBridge Demo

This is the example application from the root [README.md](../../README.md): a full desktop app (MyObject/TestObject/HelloWorld exposed to a Svelte frontend) demonstrating every WebBridge feature — properties, methods (sync + async), events, constants, and error handling. It also serves as this repo's proof that the `webbridge` library actually works when consumed like any other CMake target (see `src/CMakeLists.txt`'s `target_link_libraries(... webbridge::webbridge ...)`).

## Prerequisites

- **Visual Studio 2022** with C++ Desktop Development (MSVC compiler)
- **CMake 3.26+**
- **Python 3** (for the code generator)
- **Node.js** (for the frontend build)
- **Microsoft Edge WebView2 Runtime** (usually preinstalled on Windows 10/11)

No Conan, vcpkg, or Conda needed — every C++ dependency (both the library's and this example's) is fetched by CMake's own `FetchContent`, and the build uses the Visual Studio generator directly (no separate Ninja install or Developer Command Prompt required).

## Setup

**1. Install the code generator's Python packages**

Run this once, from the **repository root**, with whichever `python`/`pip` you want CMake to find (a plain global install is fine — no virtual environment is required, though you're welcome to use one):

```bash
pip install -r requirements.txt
```

**2. Configure and build**

`configure.bat` and `build.bat` live at the **repository root**, not in this folder — they build the `webbridge` library and this example together. Run them from the repository root:

```bash
configure.bat
```

`configure.bat` generates the CMake build (Visual Studio 17 2022 generator) for the `webbridge` library and this example together, fetching every C++ dependency via `FetchContent` along the way.

Then build:

```bash
# Build Debug (default)
build.bat

# Build Release
build.bat --release

# Rebuild (clean first)
build.bat --rebuild --release
```

**VS Code Integration:**
- **Build tasks**: Press `Ctrl+Shift+B` to access build tasks (Build, Rebuild, Clean, etc.)
- **Debugging**: Press `F5` to build and debug — launch configurations are in `.vscode/launch.json`

**Note:** The frontend (Vite + Svelte 5 + TypeScript, in `frontend/`) is built automatically as part of the CMake build. The compiled assets are embedded into the executable via CMakeRC and served over a local HTTP server by `ResourceServer`.

**3. Run the application**

Because this example is built via `add_subdirectory(examples/demo)` from the root, its build output lives one level deeper than a top-level target would:

```bash
# Debug build (with DevTools):
build\examples\demo\src\Debug\webbridge_hackathon.exe

# Release build (without DevTools):
build\examples\demo\src\Release\webbridge_hackathon.exe
```

## Benchmarks

See [doc/performance.md](doc/performance.md) for method-call and type-registration performance numbers (measured via the "Run Benchmark" button in the demo UI, backed by `TestObject`).
