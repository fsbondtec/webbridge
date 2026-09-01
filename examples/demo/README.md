# WebBridge Demo

This is the example application from the root [README.md](../../README.md): a full desktop app (MyObject/TestObject/HelloWorld exposed to a Svelte frontend) demonstrating every WebBridge feature — properties, methods (sync + async), events, constants, and error handling. It also serves as this repo's proof that the `webbridge` library actually works when consumed like any other CMake target (see `src/CMakeLists.txt`'s `target_link_libraries(... webbridge::webbridge ...)`).

## Prerequisites

- **Visual Studio 2022** with C++ Desktop Development (MSVC compiler)
- **Conan 2** (`pip install conan`)
- **CMake 3.26+**
- **Anaconda3** or Miniconda
- **Node.js** (for the frontend build)
- **Microsoft Edge WebView2 Runtime** (usually preinstalled on Windows 10/11)
- **Ninja** (included in the conda environment)

## Setup

**1. Create the Conda environment**

The environment includes Python 3.12 and the packages required by the code generator (tree-sitter, jinja2). Run this from the **repository root**:

```bash
conda env create -f environment.yml
# or, if the environment already exists:
conda env update --file environment.yml --name webbridge_hackathon --prune
```

**2. Configure and build**

`configure.bat` and `build.bat` live at the **repository root**, not in this folder — they build the `webbridge` library and this example together. Run them from the repository root:

```bash
configure.bat
```

`configure.bat` will:
- Activate the Conda environment
- Install C++ dependencies via Conan (nlohmann_json for the library; fmt, argparse, cpp-httplib, portable-file-dialogs for this example)
- Initialize the MSVC environment for Ninja
- Generate the CMake build configuration with Ninja Multi-Config for all build types (Debug, Release, RelWithDebInfo, MinSizeRel)

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
