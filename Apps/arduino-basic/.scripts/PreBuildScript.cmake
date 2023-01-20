# Copyright (c) 2020 Arduino CMake Toolchain

cmake_policy(VERSION 3.0)

set(ARDUINO_TOOLCHAIN_DIR "/home/neumann/W/Tria/rinasense/Arduino-CMake-Toolchain")
set(ARDUINO_PATTERN_NAMES 
	"hooks.prebuild.1"
	"hooks.prebuild.2"
	"hooks.prebuild.3"
	"hooks.prebuild.4"
	"hooks.prebuild.5"
	"hooks.prebuild.6"
)
set(ARDUINO_PATTERN_CMDLINE_LIST 
	"bash -c \"[ ! -f \\\"{build.source.path}\\\"/partitions.csv ] || cp -f \\\"{build.source.path}\\\"/partitions.csv \\\"{build.path}\\\"/partitions.csv\""
	"bash -c \"[ -f \\\"{build.path}\\\"/partitions.csv ] || [ ! -f \\\"/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/variants/firebeetle32\\\"/partitions.csv ] || cp \\\"/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/variants/firebeetle32\\\"/partitions.csv \\\"{build.path}\\\"/partitions.csv\""
	"bash -c \"[ -f \\\"{build.path}\\\"/partitions.csv ] || cp \\\"/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4\\\"/tools/partitions/default.csv \\\"{build.path}\\\"/partitions.csv\""
	"bash -c \"[ -f \\\"{build.source.path}\\\"/bootloader.bin ] && cp -f \\\"{build.source.path}\\\"/bootloader.bin \\\"{build.path}\\\"/{build.project_name}.bootloader.bin || ( [ -f \\\"/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/variants/firebeetle32\\\"/bootloader.bin ] && cp \\\"/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/variants/firebeetle32\\\"/bootloader.bin \\\"{build.path}\\\"/{build.project_name}.bootloader.bin || cp -f \\\"/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4\\\"/tools/sdk/esp32/bin/bootloader_dio_80m.bin \\\"{build.path}\\\"/{build.project_name}.bootloader.bin )\""
	"bash -c \"[ ! -f \\\"{build.source.path}\\\"/build_opt.h ] || cp -f \\\"{build.source.path}\\\"/build_opt.h \\\"{build.path}\\\"/build_opt.h\""
	"bash -c \"[ -f \\\"{build.path}\\\"/build_opt.h ] || touch \\\"{build.path}\\\"/build_opt.h\""
)

if ("${ARDUINO_BUILD_PATH}" STREQUAL "")
	message(FATAL_ERROR "Script not allowed to be invoked directly!!!")
endif()

list(LENGTH ARDUINO_PATTERN_NAMES _num_patterns)
set(_idx 0)
while("${_idx}" LESS "${_num_patterns}")
	list(GET ARDUINO_PATTERN_NAMES "${_idx}" _pattern_name)
	list(GET ARDUINO_PATTERN_CMDLINE_LIST "${_idx}" _pattern_cmdline)
	math(EXPR _idx "${_idx} + 1")
	string(REGEX MATCHALL "{[^{}]+}" _var_list "${_pattern_cmdline}")
	if (NOT "${_var_list}" STREQUAL "")
		list(REMOVE_DUPLICATES _var_list)
	endif()
	foreach(_var_str IN LISTS _var_list)
		string(REGEX MATCH "{(.+)}" _match "${_var_str}")
		set(_var_name "${CMAKE_MATCH_1}")
		string(MAKE_C_IDENTIFIER "ARDUINO_${_var_name}" _var_id)
		string(TOUPPER "${_var_id}" _var_id)
		if (NOT DEFINED "${_var_id}")
			message(WARNING "Variable ${_var_str} unknown in ${_pattern_name} "
				"(${_pattern_cmdline})")
		endif()
		string(REPLACE "${_var_str}" "${${_var_id}}" _pattern_cmdline
			"${_pattern_cmdline}")
	endforeach()
	separate_arguments(_pattern UNIX_COMMAND "${_pattern_cmdline}")
	if (CMAKE_VERBOSE_MAKEFILE OR DEFINED ENV{VERBOSE})
		execute_process(COMMAND
			# ${CMAKE_COMMAND} -E env CLICOLOR_FORCE=1
			# ${CMAKE_COMMAND} -E cmake_echo_color --blue
			${CMAKE_COMMAND} -E echo
			"Executing ${_pattern_name}")
		execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${_pattern_cmdline}")
	endif()
	execute_process(COMMAND ${_pattern} RESULT_VARIABLE result)
	if (NOT "${result}" EQUAL 0)
		message(FATAL_ERROR "${_pattern_name} failed!!!")
	endif()
endwhile()

