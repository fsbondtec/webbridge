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

## Repository layout

- `src/webbridge/` — the library itself (this is what you'd add to your own project)
- `cmake/webbridge.cmake` — the `webbridge_generate()` CMake function that drives the code generator
- `tools/` — the Python/tree-sitter code generator
- `examples/demo/` — the full example application (C++ + Svelte frontend) exercising every feature; see [examples/demo/README.md](examples/demo/README.md) to build and run it

## Getting started

This repo contains **webbridge** itself, which can be used as an external library. Also an examples folder is provided, which shows a demo.

### Prerequisites

- **Visual Studio 2022** with C++ Desktop Development (MSVC compiler)
- **CMake 3.26+**
- **Python 3** (for the code generator)
- **Node.js** (for the frontend build)
- **Microsoft Edge WebView2 Runtime** (usually preinstalled on Windows 10/11)

To use WebBridge, no Conan or vcpkg needed. All C++/Python dependencies are fetched and installed automatically by CMake. 
Pulling webbridge in via `FetchContent` only fetches *its own* dependencies, not the demo's extra ones. 
This repo also ships a `CMakePresets.json`, so `cmake --preset windows-vs2022` works without needing to know the exact generator/architecture flags.

### Using it in your own C++ project

Add this to your project's `CMakeLists.txt` to fetch from the git repository:

```cmake
include(FetchContent)
FetchContent_Declare(
    webbridge
    GIT_REPOSITORY https://github.com/fsbondtec/webbridge.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(webbridge)

target_link_libraries(your_target PRIVATE webbridge::webbridge)
```

Also add this webbridge generator to your `CMakeLists.txt` :
```cmake
webbridge_generate(
    TARGET your_target
    AUTO
    LANGUAGE cpp
)
```

Replace `your_target` with the name of your own CMake target.

In your project:
1. Write a class that inherits from `webbridge::object` — see [Minimal Example](#minimal-example) below for what this looks like.

2. Register it where you create your webview window:
```cpp
webbridge::register_type<YourClass>(&your_webview);
```

First build with:

```bash
cmake -B build -S .
```
Then choose a build variant:
```bash
cmake --build build --config Debug                      # Debug
cmake --build build --config Release                    # Release
cmake --build build --config Release --clean-first      # Clean rebuild
```
And executing with:
```bash
# Debug build (with DevTools):
build\Debug\your_target.exe
# Release build (without DevTools):
build\Release\your_target.exe
```

### Running the demo
If you cloned the repository you can test the demo.
Open a terminal **in this folder** and run:

```bash
configure.bat
build.bat
```

`configure.bat` downloads everything else it needs automatically (no separate install steps, no accounts, nothing to configure by hand). Once `build.bat` finishes, launch the app:

```bash
build\examples\demo\src\Debug\webbridge_hackathon.exe
```

For Release builds, troubleshooting, or what each step actually does, see [examples/demo/README.md](examples/demo/README.md).


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
