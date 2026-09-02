# C++ to JavaScript Bridge [webbridge]

This repository is a demonstration project created during the fsbondtec [Christmas Hackathon 2025](https://www.fsbondtec.at/).

C++ objects are seamlessly integrated into modern web applications as a modern **Qt alternative** for web-based UIs. While Qt uses QML/Qt Quick or Qt WebEngine for GUI development, WebBridge leverages standard web technologies. 

**Motivation:** A key advantage over Qt is the significantly reduced boilerplate code. In Qt, a simple property requires extensive boilerplate with getter, setter, signal, and backing field:

```cpp
// Qt approach - verbose boilerplate
Q_PROPERTY(bool aBool READ aBool WRITE setABool NOTIFY aBoolChanged)
bool aBool() const {
    return _aBool;
}
void setABool(bool v) {
    if (v != _aBool) {
        _aBool = v;
        emit aBoolChanged();
    }
}
signals:
    void aBoolChanged();
private:
    bool _aBool;

// WebBridge approach - minimal and clean
property<bool> aBool;
```

The solution is based on **webview** (C++ wrapper for Microsoft WebView2/Chromium) and a **Python code generator** (`tools/generate.py`) that uses **tree-sitter** to analyze C++ classes and automatically generate C++ registration headers and TypeScript type definitions. The build process automatically invokes the code generator via CMake, making the workflow seamless. Code generation is required because C++26 reflection is not yet available.

## Quick Start

This repo contains two things: **webbridge itself** (a small C++ library, `src/webbridge/`) and **a full example app** built with it (`examples/demo/`), so you can see it working before deciding to use it. Pick whichever you're here for:

### Option A — Just want to see it run?

Install these four things first (all free, all standard tools — skip anything you already have):

| Tool | Why you need it | Get it |
|---|---|---|
| Visual Studio 2022 | The C++ compiler (install the **"Desktop development with C++"** workload) | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/) |
| CMake 3.26+ | Drives the whole build | [cmake.org/download](https://cmake.org/download/) |
| Python 3 | Just needs to be on your PATH — its packages install themselves automatically, nothing to do by hand | [python.org/downloads](https://www.python.org/downloads/) |
| Node.js | Builds the demo's web UI | [nodejs.org](https://nodejs.org/) |

Then open a terminal **in this folder** and run:

```bash
configure.bat
build.bat
```

`configure.bat` downloads everything else it needs automatically (no separate install steps, no accounts, nothing to configure by hand). Once `build.bat` finishes, launch the app:

```bash
build\examples\demo\src\Debug\webbridge_hackathon.exe
```

That's it. For Release builds, troubleshooting, or what each step actually does, see [examples/demo/README.md](examples/demo/README.md).

### Option B — Want to add webbridge to your own C++ project?

You don't need to download anything by hand, or even find a browser and click a download button. Just add this to your own project's `CMakeLists.txt` — it tells CMake to fetch webbridge automatically the next time you configure your project (this is a standard, built-in CMake feature called `FetchContent`, not anything webbridge-specific):

```cmake
include(FetchContent)
FetchContent_Declare(
    webbridge
    GIT_REPOSITORY https://github.com/fsbondtec/webbridge-hackathon.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(webbridge)

target_link_libraries(your_target PRIVATE webbridge::webbridge)
```

You'll need the same first three tools from the table above (Visual Studio 2022, CMake 3.26+, Python 3 on PATH) — Node.js is only needed if you also want a web frontend like the demo has. Everything else — webbridge's own C++ dependencies, and the Python packages its code generator needs — is downloaded and installed automatically the first time you configure. No accounts, no extra package managers, no manual `pip install`.

Once it's linked in, exposing one of your own classes to JavaScript takes three steps:

1. Write a class that inherits from `webbridge::object` — see [Minimal Example](#minimal-example) below for what this looks like.
2. Tell CMake to generate the glue code for it:
   ```cmake
   webbridge_generate(
       TARGET your_target
       AUTO
       LANGUAGE cpp
   )
   ```
3. Register it where you create your webview window:
   ```cpp
   webbridge::register_type<YourClass>(&your_webview);
   ```

CMake re-generates the glue code automatically whenever you change the class — nothing to re-run by hand.

Building and running is nothing webbridge-specific at this point — it's your own project:

```bash
cmake -B build -S .
cmake --build build --config Debug
build\Debug\your_target.exe
```


## Repository layout

- `src/webbridge/` — the library itself (this is what you'd add to your own project)
- `cmake/webbridge.cmake` — the `webbridge_generate()` CMake function that drives the code generator
- `tools/` — the Python/tree-sitter code generator
- `examples/demo/` — the full example application (C++ + Svelte frontend) exercising every feature; see [examples/demo/README.md](examples/demo/README.md) to build and run it

## Building this repository (more detail)

Everything in [Quick Start](#quick-start) above is all you need day-to-day. This section is for anyone who wants to understand or customize the raw commands `configure.bat`/`build.bat` run under the hood — useful if you're contributing to webbridge itself, or scripting a CI pipeline.

The checked-in `CMakePresets.json` means you never need to type out the exact generator/architecture flags yourself — a "preset" is just a named, saved set of CMake options:

```bash
cmake --preset windows-vs2022
cmake --build --preset windows-vs2022-debug
# or: cmake --build --preset windows-vs2022-release
```

If you'd rather not use presets, the equivalent raw commands are:

```bash
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

Either way, the built demo exe ends up at `build\examples\demo\src\Debug\webbridge_hackathon.exe` (or `...\Release\...` for a Release build) — run it directly, same as in Quick Start.

Running `configure.bat`/`build.bat` from the repository root builds the `webbridge` library **and** the example application together (`WEBBRIDGE_BUILD_EXAMPLES` defaults to on when this repo is the top-level project, and off when you pull webbridge into your own project per Option B above).

## Concepts

Every class to be exposed to the web must inherit from `webbridge::object`. The API is inspired by Qt and provides the following mechanisms for JavaScript integration:

* **Methods** – Public C++ methods are automatically available in JavaScript (similar to Qt's Q_INVOKABLE)
* **Properties** – Exposed as Svelte-compatible stores (read-only, inspired by Qt's Q_PROPERTY)
* **Events** – Trigger custom event listeners in JavaScript (equivalent to Qt signals)
* **Constants** - Readonly JS values

The automatically generated code is functionally inspired by Qt and Qt's MOC (Meta-Object Compiler), but the WebBridge classes require significantly less boilerplate code than their Qt equivalents.

### Methods

All public methods of a `webbridge::object` class are automatically published to JavaScript.

A function marked with the `[[async]]` attribute is executed in a separate worker thread. This prevents blocking the main thread on the C++ side. On the JavaScript side, both synchronous and asynchronous methods always return a `Promise` and never block the main thread.

### Properties

Properties are similar to primitive data types but require access via the parenthesis operator `()` in C++. In JavaScript, properties are exposed as Svelte-compatible, reactive stores. They are read-only in JavaScript; changes to the property value in C++ are automatically and immediately propagated to JavaScript.

### Events

Events are the WebBridge equivalent of the Qt signal/slot mechanism.

### Constants

WebBridge supports exposing constants as both **static** (class-wide) and **non-static** (instance-specific). Both variants are automatically exported to JavaScript and are available there as read-only values.

### Error Handling

WebBridge implements robust error handling, distinguishing between JavaScript client errors (4xxx) and C++ server errors (5xxx). Errors are serialized as JSON objects and, for asynchronous operations, are propagated as rejected Promises.

**Error format:**
```json
{
  "error": {
    "code": 4001,
    "message": "Invalid argument type",
    "details": { "param": "value", "expected": "string" },
    "stack": "at function (file.js:10:5)",
    "origin": "javascript"
  }
}
```

**Error codes:**
- `4000-4999`: JavaScript errors (e.g., 4001 = JSON_PARSE_ERROR during parameter deserialization)
- `5000-5999`: C++ errors (e.g., 5000 = RUNTIME_ERROR during runtime errors)

Inspired by JSON-RPC 2.0, GraphQL, and HTTP status codes. Promises are automatically rejected on error, enabling clean exception handling with async/await syntax.

## Minimal Example

The following example shows how to define a C++ class with methods, properties, and events for web integration with WebBridge.

```cpp
#include "webbridge/object.h"

class MyObject : public webbridge::object
{
public:
    property<bool> aBool = false;
    property<std::string> strProp;
    event<int, bool> aEvent;
    inline static constexpr auto PI = 3.141592654;
    const std::string version = "1.0";

public:
    [[async]] void foo(std::string_view val) {
        // long-running action
        strProp = val;
        aEvent.emit(42, false);
    }

    bool bar() const {
        // Parenthesis operator accesses value
        return !aBool();
    }
};
```

### Tracking JavaScript Properties

```js
const myObj = await MyObject.create();
// ...
myObj.aBool.subscribe(value => {
    console.log('aBool updated:', value);
});
```

### Calling a C++ Method from JavaScript

```js
const myObj = await MyObject.create();

// Example: call a synchronous method
// Blocks the main thread in C++, but JavaScript waits asynchronously
const result = await myObj.bar();
console.log('Result of bar():', result);

// Example: call an asynchronous method ([[async]] = worker thread in C++)
// Does not block the main thread in either C++ or JavaScript
myObj.foo('new value').then(() => {
    console.log('foo() completed');
});
```

### Handling Events in JavaScript

```js
const myObj = await MyObject.create();

// Register event listener (similar to Node.js EventEmitter)
myObj.aEvent.on((intValue, boolValue) => {
    console.log('Event received:', intValue, boolValue);
});

// Alternatively, one-time event
myObj.aEvent.once((intValue, boolValue) => {
    console.log('One-time event:', intValue, boolValue);
});
```

### Accessing Constants in JavaScript

```js
const myObj = await MyObject.create();
console.log(myObj.version); // Instance constant: "1.0"
console.log(MyObject.PI);   // Static constant: 3.141592654
```


## Registration

To make C++ classes available in JavaScript, they must be explicitly registered. The code generator creates the necessary binding files, which are then included in CMake. In your `main.cpp`, you must call the generated registration function:


```cpp
#include "MyObject_registration.h"

int main() {
    // Register the class for JavaScript
    webbridge::register_type<MyObject>();

    // ... initialize and run your webview ...
}
```

## Known Limitations

The current implementation has the following limitations:

- Overloaded constructors and methods are not supported.
- Enums are automatically detected and exported to TypeScript, but complex enum use cases may require additional handling.
- Currently Windows-only (relies on Microsoft WebView2).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Third-party licenses can be found in [THIRD-PARTY-NOTICES.txt](THIRD-PARTY-NOTICES.txt).
