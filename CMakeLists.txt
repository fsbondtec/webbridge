cmake_minimum_required(VERSION 3.26)

# Must be set before project() to control the MSVC runtime library selection
# below (previously handled for us by Conan's generated toolchain file).
cmake_policy(SET CMP0091 NEW)

# CMAKE_CURRENT_SOURCE_DIR (not CMAKE_SOURCE_DIR) so this still finds
# cmake/webbridge.cmake when webbridge is add_subdirectory()'d/FetchContent'd
# from another project - CMAKE_SOURCE_DIR would then point at the consumer's
# root instead of this one.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

project(
	webbridge
	VERSION 1.0.0
	LANGUAGES CXX C
)

# Match the dynamic (DLL) MSVC runtime that webview/WebView2 expect.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

include(webbridge)
include(FetchContent)

find_package(Python REQUIRED COMPONENTS Interpreter)

# Both dependencies are fetched by CMake itself (no Conan/vcpkg/system install
# required) so the library builds with nothing but CMake + a C++20 compiler.
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
	nlohmann_json
	GIT_REPOSITORY https://github.com/nlohmann/json
	GIT_TAG v3.11.3
	GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
	webview
	GIT_REPOSITORY https://github.com/webview/webview
	GIT_TAG 0.12.0)
FetchContent_MakeAvailable(webview)

add_library(webbridge STATIC
	src/webbridge/object.h
	src/webbridge/error.h
	src/webbridge/impl/binding_helpers.h
	src/webbridge/impl/concepts.h
	src/webbridge/impl/dispatcher.h
	src/webbridge/impl/error_handler.h
	src/webbridge/impl/error_handler.cpp
	src/webbridge/impl/event_impl.h
	src/webbridge/impl/object_registry.h
	src/webbridge/impl/property_impl.h
	src/webbridge/impl/thread_pool.h
	src/webbridge/impl/thread_pool.cpp
	src/webbridge/impl/type_registration.h
	src/webbridge/impl/type_registration.cpp
)
add_library(webbridge::webbridge ALIAS webbridge)

target_include_directories(webbridge PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_features(webbridge PUBLIC cxx_std_20)

# WebView2 (via the webview library) requires a modern Windows SDK target;
# PUBLIC because consumers including webbridge headers pull in webview.h too.
target_compile_definitions(webbridge PUBLIC _WIN32_WINNT=0x0A00)

target_link_libraries(webbridge PUBLIC
	nlohmann_json::nlohmann_json
	webview::core
)

if(MSVC)
	target_compile_options(webbridge PRIVATE /W3 /bigobj)
endif()

# Building the example application is opt-out when this is the top-level
# project (e.g. cloned standalone) and opt-in when webbridge is consumed via
# add_subdirectory()/FetchContent from another project.
option(WEBBRIDGE_BUILD_EXAMPLES "Build the webbridge example application" ${PROJECT_IS_TOP_LEVEL})

if(WEBBRIDGE_BUILD_EXAMPLES)
	add_subdirectory(examples/demo)
endif()
