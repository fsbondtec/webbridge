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

set(FMT_INSTALL OFF CACHE INTERNAL "")
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt
	GIT_TAG 11.0.2
	GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(fmt)

set(HTTPLIB_COMPILE OFF CACHE INTERNAL "")
FetchContent_Declare(
	httplib
	GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
	GIT_TAG v0.18.1
	GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
	portable_file_dialogs
	GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs
	GIT_TAG 0.1.0
	GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(portable_file_dialogs)

add_subdirectory(frontend)
add_subdirectory(src)
