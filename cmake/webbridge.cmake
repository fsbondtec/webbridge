get_filename_component(WEBBRIDGE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# Helper function to parse discoverer output
function(_parse_discoverer_output discoverer_output out_var)
	set(result)
	if(discoverer_output)
		string(REPLACE "\n" ";" lines "${discoverer_output}")
		foreach(line ${lines})
			if(line)
				string(REPLACE "|" ";" parts "${line}")
				list(GET parts 0 file)
				list(GET parts 1 class_name)
				list(APPEND result "${file}|${class_name}")
			endif()
		endforeach()
	endif()
	set(${out_var} ${result} PARENT_SCOPE)
endfunction()

function(webbridge_generate)
	# Check Python availability
	if(NOT Python_EXECUTABLE)
		find_package(Python REQUIRED)
	endif()

	set(options AUTO)
	set(oneValueArgs TARGET OUTPUT_DIR LANGUAGE FILE CLASS_NAME)
	set(multiValueArgs FILES)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		"${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	# Validate parameter combinations
	if(arg_FILE AND arg_FILES)
		message(FATAL_ERROR "webbridge_generate: Use either FILE or FILES, not both")
	endif()

	if(arg_FILE AND arg_AUTO)
		message(FATAL_ERROR "webbridge_generate: FILE cannot be used with AUTO")
	endif()

	if(arg_AUTO AND arg_FILES)
		message(FATAL_ERROR "webbridge_generate: Use either AUTO or FILES, not both")
	endif()

	if(arg_FILE AND NOT arg_CLASS_NAME)
		message(FATAL_ERROR "webbridge_generate: FILE requires CLASS_NAME")
	endif()

	if(arg_CLASS_NAME AND NOT arg_FILE)
		message(FATAL_ERROR "webbridge_generate: CLASS_NAME requires FILE")
	endif()

	if(NOT arg_AUTO AND NOT arg_FILES AND NOT arg_FILE)
		message(FATAL_ERROR "webbridge_generate: Either AUTO, FILES, or FILE must be specified")
	endif()

	# Default to CMAKE_CURRENT_BINARY_DIR if not specified
	if(NOT arg_OUTPUT_DIR)
		set(arg_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	# Default to cpp if not specified
	if(NOT arg_LANGUAGE)
		set(arg_LANGUAGE cpp)
	endif()

	# Collect files to process
	set(all_files)

	# Explicit FILE + CLASS_NAME mode (no discovery)
	if(arg_FILE)
		get_filename_component(abs_file "${arg_FILE}" ABSOLUTE)
		list(APPEND all_files "${abs_file}|${arg_CLASS_NAME}")
	# Collect auto-detected header files from target if AUTO flag is set
	elseif(arg_AUTO)
		set(header_files)
		get_target_property(target_sources ${arg_TARGET} SOURCES)
		if(target_sources)
			foreach(source ${target_sources})
				# Get absolute path
				get_filename_component(abs_source "${source}" ABSOLUTE)
				# Check if it's a header file
				if(abs_source MATCHES "\\.(h|hpp)$")
					list(APPEND header_files ${abs_source})
				endif()
			endforeach()
		endif()

		if(header_files)
			execute_process(
				COMMAND ${Python_EXECUTABLE}
				${WEBBRIDGE_ROOT_DIR}/tools/discoverer.py
					${header_files}
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				OUTPUT_VARIABLE discoverer_output
				OUTPUT_STRIP_TRAILING_WHITESPACE
				RESULT_VARIABLE result
			)

			if(result AND NOT result EQUAL 0)
				message(FATAL_ERROR "discoverer.py failed with exit code ${result}")
			endif()

			# Parse output using helper function
			_parse_discoverer_output("${discoverer_output}" all_files)
		endif()
	# Process explicit FILES - discover classes in these files
	elseif(arg_FILES)
		set(abs_files)
		foreach(file ${arg_FILES})
			get_filename_component(abs_file "${file}" ABSOLUTE)
			list(APPEND abs_files ${abs_file})
		endforeach()

		execute_process(
			COMMAND ${Python_EXECUTABLE}
			${WEBBRIDGE_ROOT_DIR}/tools/discoverer.py
				${abs_files}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE discoverer_output
			OUTPUT_STRIP_TRAILING_WHITESPACE
			RESULT_VARIABLE result
		)

		if(result AND NOT result EQUAL 0)
			message(FATAL_ERROR "discoverer.py failed with exit code ${result}")
		endif()

		# Parse output using helper function
		_parse_discoverer_output("${discoverer_output}" all_files)
	endif()

	# Check if we have any files to process
	if(NOT all_files)
		if(arg_AUTO)
			message(WARNING "webbridge_generate: No files with webbridge::object classes found in target ${arg_TARGET}")
		else()
			message(WARNING "webbridge_generate: No files to process")
		endif()
		return()
	endif()

	# Prepare batch arguments and output files
	set(batch_args)
	set(all_output_files)
	set(all_input_files)
	
	foreach(pair ${all_files})
		# Split pair into file and class_name
		string(REPLACE "|" ";" parts "${pair}")
		list(GET parts 0 file)
		list(GET parts 1 class_name)
		
		# Add to batch arguments in format "file|classname"
		list(APPEND batch_args "${file}|${class_name}")
		list(APPEND all_input_files ${file})
		
		# Collect output files based on LANGUAGE
		if(arg_LANGUAGE STREQUAL "cpp")
			list(APPEND all_output_files "${arg_OUTPUT_DIR}/${class_name}_registration.h")
			list(APPEND all_output_files "${arg_OUTPUT_DIR}/${class_name}_registration.cpp")
		elseif(arg_LANGUAGE STREQUAL "ts-impl")
			list(APPEND all_output_files "${arg_OUTPUT_DIR}/${class_name}.ts")
		endif()
	endforeach()

	# Determine template files and python args based on LANGUAGE
	if(arg_LANGUAGE STREQUAL "cpp")
		set(template_files 
			"${WEBBRIDGE_ROOT_DIR}/tools/templates/registration_header.h.j2"
			"${WEBBRIDGE_ROOT_DIR}/tools/templates/registration_impl.cpp.j2"
		)
		set(python_out_arg --cpp_out)
	elseif(arg_LANGUAGE STREQUAL "ts-impl")
		set(template_files "${WEBBRIDGE_ROOT_DIR}/tools/templates/impl.ts.j2")
		set(python_out_arg --ts_impl_out)
	else()
		message(FATAL_ERROR "Invalid LANGUAGE: ${arg_LANGUAGE}. Must be 'cpp' or 'ts-impl'")
	endif()

	# Single batch command for all files
	list(LENGTH all_files file_count)
	add_custom_command(
		OUTPUT ${all_output_files}
		COMMAND ${Python_EXECUTABLE}
			${WEBBRIDGE_ROOT_DIR}/tools/generate.py
			--batch
			${batch_args}
			${python_out_arg}
			${arg_OUTPUT_DIR}
		DEPENDS
			${WEBBRIDGE_ROOT_DIR}/tools/generate.py
			${WEBBRIDGE_ROOT_DIR}/tools/parser.py
			${WEBBRIDGE_ROOT_DIR}/tools/tstypes.py
			${template_files}
			${all_input_files}
		COMMENT "Generating ${arg_LANGUAGE} (batch of ${file_count} files)"
		VERBATIM
	)

	# For C++, add generated files to target and set up include paths
	if(arg_LANGUAGE STREQUAL "cpp")
		foreach(output_file ${all_output_files})
			set_source_files_properties(${output_file} PROPERTIES GENERATED TRUE)
		endforeach()
		target_sources(${arg_TARGET} PRIVATE ${all_output_files})
		
		# Add generated header directory to include path
		target_include_directories(${arg_TARGET} PRIVATE ${arg_OUTPUT_DIR})
	endif()
endfunction()
