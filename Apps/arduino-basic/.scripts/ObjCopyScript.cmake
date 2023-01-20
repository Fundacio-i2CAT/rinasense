# Copyright (c) 2020 Arduino CMake Toolchain

cmake_policy(VERSION 3.0)

set(ARDUINO_TOOLCHAIN_DIR "/home/neumann/W/Tria/rinasense/Arduino-CMake-Toolchain")
set(ARDUINO_PATTERN_NAMES 
	"objcopy.partitions.bin"
	"objcopy.bin"
)
set(ARDUINO_PATTERN_CMDLINE_LIST 
	"python3 /home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/tools/gen_esp32part.py -q \"{build.path}/partitions.csv\" \"{build.path}/{build.project_name}.partitions.bin\""
	"python3 /home/neumann/.arduino15/packages/esp32/tools/esptool_py/3.3.0/esptool.py --chip esp32 elf2image --flash_mode dio --flash_freq 80m --flash_size 4MB -o \"{build.path}/{build.project_name}.bin\" \"{build.path}/{build.project_name}.elf\""
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

