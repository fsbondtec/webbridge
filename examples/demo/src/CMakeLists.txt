# fmt, httplib, portable_file_dialogs, and (transitively via webbridge)
# nlohmann_json are all FetchContent'd by examples/demo/CMakeLists.txt /
# root CMakeLists.txt - do not find_package() any of them here, that would
# collide with the already-defined targets of the same name.

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(webbridge_hackathon_SOURCES
	main.cpp
	MyObject.cpp
	TestObject.cpp
	HelloWorld.cpp
	ResourceServer.h
)

set(webbridge_hackathon_TO_REGISTRATION_SOURCES
	MyObject.h
	TestObject.h
	HelloWorld.h
)

add_executable(webbridge_hackathon WIN32
	${webbridge_hackathon_SOURCES}
	${webbridge_hackathon_TO_REGISTRATION_SOURCES}
)

target_compile_features(webbridge_hackathon PRIVATE cxx_std_20)

target_link_libraries(webbridge_hackathon PRIVATE
	webbridge::webbridge
	fmt::fmt
	nlohmann_json::nlohmann_json
	httplib::httplib
	portable_file_dialogs
	frontend_resources
)

target_precompile_headers(webbridge_hackathon PRIVATE pch.h)

webbridge_generate(
	TARGET webbridge_hackathon
	AUTO
	LANGUAGE cpp
)

target_compile_options(webbridge_hackathon PRIVATE /W3 /bigobj)

add_custom_command(TARGET webbridge_hackathon POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin

	COMMAND ${CMAKE_COMMAND} -E
	$<$<CONFIG:Debug>:echo> $<$<CONFIG:Debug>:"skip command in Debug mode: ">
	copy_directory_if_different $<TARGET_FILE_DIR:webbridge_hackathon> "${CMAKE_SOURCE_DIR}/bin"
)
