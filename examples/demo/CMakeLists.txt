# Full demo application showing how to consume the webbridge library:
# MyObject/TestObject/HelloWorld exposed to a Svelte frontend, embedded into
# the executable via cmrc and served locally over HTTP.
#
# Only ever reached via the root CMakeLists.txt's
# `add_subdirectory(examples/demo)` (gated by WEBBRIDGE_BUILD_EXAMPLES) - not
# meant to be configured as a standalone CMake source directory.

FetchContent_Declare(
	cmrc
	GIT_REPOSITORY https://github.com/vector-of-bool/cmrc.git
	GIT_TAG master
)
FetchContent_MakeAvailable(cmrc)

add_subdirectory(frontend)
add_subdirectory(src)
