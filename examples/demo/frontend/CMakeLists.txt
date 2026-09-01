find_program(NPM_EXECUTABLE npm REQUIRED)
message(STATUS "Found npm: ${NPM_EXECUTABLE}")

# Generate TypeScript implementation files for C++ classes.
# Resolved relative to this file (examples/demo/frontend/), not CMAKE_SOURCE_DIR,
# since CMAKE_SOURCE_DIR is the webbridge library's own root, not this example's.
set(WEBBRIDGE_HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../src/MyObject.h
	${CMAKE_CURRENT_SOURCE_DIR}/../src/TestObject.h
	${CMAKE_CURRENT_SOURCE_DIR}/../src/HelloWorld.h
)

# Output to frontend/src/generated directory (gitignored)
set(WEBBRIDGE_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/generated)

# Ensure output directory exists
file(MAKE_DIRECTORY ${WEBBRIDGE_OUTPUT_DIR})

# Prepare list of generated files and batch arguments for generate.py
set(GENERATED_TS_FILES)
set(BATCH_ARGS)

foreach(header_file IN LISTS WEBBRIDGE_HEADER_FILES)
	get_filename_component(class_name ${header_file} NAME_WE)
	list(APPEND GENERATED_TS_FILES "${WEBBRIDGE_OUTPUT_DIR}/${class_name}.ts")
	list(APPEND BATCH_ARGS "${header_file}|${class_name}")
endforeach()

# Generate TypeScript files at configuration time if they don't exist
set(NEED_GENERATION FALSE)
foreach(ts_file IN LISTS GENERATED_TS_FILES)
	if(NOT EXISTS ${ts_file})
		set(NEED_GENERATION TRUE)
		break()
	endif()
endforeach()

if(NEED_GENERATION)
	message(STATUS "Generating TypeScript implementations at configuration time...")
	execute_process(
		COMMAND ${Python_EXECUTABLE}
			${CMAKE_SOURCE_DIR}/tools/generate.py
			--batch
			${BATCH_ARGS}
			--ts_impl_out
			${WEBBRIDGE_OUTPUT_DIR}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		RESULT_VARIABLE gen_result
		OUTPUT_VARIABLE gen_output
		ERROR_VARIABLE gen_error
	)
	if(NOT gen_result EQUAL 0)
		message(STATUS "Generation output: ${gen_output}")
		message(STATUS "Generation error: ${gen_error}")
		message(FATAL_ERROR "TypeScript generation failed with: ${gen_result}")
	endif()
	message(STATUS "TypeScript implementations generated successfully")
endif()

# Generate TypeScript implementations using discoverer (for build-time updates)
webbridge_generate(
	TARGET webbridge_gen
	FILES ${WEBBRIDGE_HEADER_FILES}
	LANGUAGE ts-impl
	OUTPUT_DIR ${WEBBRIDGE_OUTPUT_DIR}
)

add_custom_target(webbridge_gen ALL
	DEPENDS ${GENERATED_TS_FILES}
	COMMENT "Generating TypeScript implementations from C++ headers"
)

set(NPM_INSTALL_STAMP ${CMAKE_CURRENT_BINARY_DIR}/npm_install.stamp)

add_custom_command(
	OUTPUT ${NPM_INSTALL_STAMP}
	COMMAND ${NPM_EXECUTABLE} install
	COMMAND ${CMAKE_COMMAND} -E touch ${NPM_INSTALL_STAMP}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/package.json
	COMMENT "Installing npm dependencies"
)

add_custom_target(npm_install
	DEPENDS ${NPM_INSTALL_STAMP}
)

set(ASSETS
	bild.jpg
)

set(FRONTEND_SRC_FILES
	src/app.css
	src/App.svelte
	src/main.ts
)

foreach(asset IN LISTS ASSETS)
	list(APPEND FRONTEND_SRC_FILES "src/assets/${asset}")
endforeach()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/index.html)
	message(STATUS "Frontend dist not found, building now...")
	execute_process(
		COMMAND cmd /c "${NPM_EXECUTABLE}" install
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE npm_install_result
		OUTPUT_VARIABLE npm_install_output
		ERROR_VARIABLE npm_install_error
	)
	if(NOT npm_install_result EQUAL 0)
		message(STATUS "npm install output: ${npm_install_output}")
		message(STATUS "npm install error: ${npm_install_error}")
		message(FATAL_ERROR "npm install failed with: ${npm_install_result}")
	endif()
	execute_process(
		COMMAND cmd /c "${NPM_EXECUTABLE}" run build
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE npm_build_result
		OUTPUT_VARIABLE npm_build_output
		ERROR_VARIABLE npm_build_error
	)
	if(NOT npm_build_result EQUAL 0)
		message(STATUS "npm build output: ${npm_build_output}")
		message(STATUS "npm build error: ${npm_build_error}")
		message(FATAL_ERROR "npm run build failed with: ${npm_build_result}")
	endif()
	message(STATUS "Frontend build completed")
endif()

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/dist/index.html
	COMMAND ${NPM_EXECUTABLE} run build
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS
		npm_install
		vite.config.ts
		svelte.config.js
		tsconfig.json
		tsconfig.app.json
		tsconfig.node.json
		index.html
		${FRONTEND_SRC_FILES}
		${GENERATED_TS_FILES}
	COMMENT "Building Svelte frontend"
)

add_custom_target(frontend_build ALL
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dist/index.html
)

add_dependencies(frontend_build npm_install webbridge_gen)

set(FRONTEND_DIST_FILES
	dist/assets/index.css
	dist/assets/index.js
	dist/index.html
)

foreach(asset IN LISTS ASSETS)
	list(APPEND FRONTEND_DIST_FILES "dist/assets/${asset}")
endforeach()

cmrc_add_resource_library(
	frontend_resources
	NAMESPACE frontend
	WHENCE ${CMAKE_CURRENT_SOURCE_DIR}/dist
	${FRONTEND_DIST_FILES}
)

add_dependencies(frontend_resources frontend_build)
