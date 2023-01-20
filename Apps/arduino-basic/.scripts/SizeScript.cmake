# Copyright (c) 2020 Arduino CMake Toolchain

# Generated content from the selected board
set(RECIPE_SIZE_PATTERN "\"/home/neumann/.arduino15/packages/esp32/tools/xtensa-esp32-elf-gcc/gcc8_4_0-esp-2021r2-patch3/bin/xtensa-esp32-elf-size\" -A \"{build.path}/{build.project_name}.elf\"")

# Resolve the target specific values
set(cmd_pattern "${RECIPE_SIZE_PATTERN}")
string(REPLACE "{build.project_name}" "${ARDUINO_BUILD_PROJECT_NAME}"
	cmd_pattern "${cmd_pattern}")
string(REPLACE "{build.path}" "${ARDUINO_BUILD_PATH}"
	cmd_pattern "${cmd_pattern}")
string(REPLACE "{build.source.path}" "${ARDUINO_BUILD_SOURCE_PATH}"
	cmd_pattern "${cmd_pattern}")

if ("${ARDUINO_BUILD_PATH}" STREQUAL "")
	message(FATAL_ERROR "Script not allowed to be invoked directly!!!")
endif()

# Print the command line for VERBOSE mode
if (CMAKE_VERBOSE_MAKEFILE OR DEFINED ENV{VERBOSE})
	execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${cmd_pattern}")
endif()

message("################## Size Summary ##################")
separate_arguments(_pattern UNIX_COMMAND "${cmd_pattern}")
execute_process(COMMAND ${_pattern} RESULT_VARIABLE result 
	OUTPUT_VARIABLE SizeRecipeOutput)
string(REPLACE "\n" ";" SizeRecipeOutput "${SizeRecipeOutput}")

set (size_regex_list "^(\\.iram0\\.text|\\.iram0\\.vectors|\\.dram0\\.data|\\.flash\\.text|\\.flash\\.rodata)[ 	]+([0-9]+).*;^(\\.dram0\\.data|\\.dram0\\.bss|\\.noinit)[ 	]+([0-9]+).*")
set (size_name_list "program;data")
set (maximum_size_list "1310720;327680")
set (size_match_idx_list "2;2")

# For each of the elements whose size is to be printed
math(EXPR _last_index "2 - 1")
set(is_size_exeeded FALSE)
foreach(idx RANGE "${_last_index}")

	list(GET size_regex_list ${idx} size_regex)
	list(GET size_name_list ${idx} size_name)
	list(GET maximum_size_list ${idx} maximum_size)
	list(GET size_match_idx_list ${idx} size_match_idx)

	# Grep for the size in the output and add them all together
	set(tot_size 0)
	foreach(line IN LISTS SizeRecipeOutput)
    	string(REGEX MATCH "${size_regex}" match "${line}")
	    if (match)
    	    math(EXPR tot_size "${tot_size}+${CMAKE_MATCH_${size_match_idx}}")
    	endif()
	endforeach()


	# Capitalize first letter of element name
	string(SUBSTRING "${size_name}" 0 1 first_letter)
	string(SUBSTRING "${size_name}" 1 -1 rem_letters)
	string(TOUPPER "${first_letter}" first_letter)
	set(size_name "${first_letter}${rem_letters}")

	# Print total size of the element
	if (maximum_size GREATER 0)
		if (tot_size GREATER maximum_size)
			set(is_size_exeeded TRUE)
		endif()
		math(EXPR tot_size_percent "${tot_size} * 100 / ${maximum_size}")
		message("${size_name} Size: ${tot_size} of ${maximum_size} bytes "
			"(${tot_size_percent}%)")
	else()
		message("${size_name} Size: ${tot_size} bytes")
	endif()

endforeach()

message("##################################################")

if (is_size_exeeded)
	message(FATAL_ERROR "Program exceeded the maximum size!!!")
endif()

