# Copyright (c) 2020 Arduino CMake Toolchain

cmake_policy(VERSION 3.1)

# Generated content based on the selected board
set(ARDUINO_TOOLCHAIN_DIR "/home/neumann/W/Tria/rinasense/Arduino-CMake-Toolchain")
if (NOT DEFINED ARDUINO_BINARY_DIR)
	set(ARDUINO_BINARY_DIR "/home/neumann/W/Tria/rinasense/Apps/arduino-basic")
endif()
set(ARDUINO_TOOL_NAMES 
	"esptool_py"
)
set(ARDUINO_TOOL_DESCRIPTIONS
	"esptool_py (Serial Port)"
)
set(ARDUINO_TOOL_CMDLINE_LIST 
	"python3 /home/neumann/.arduino15/packages/esp32/tools/esptool_py/3.3.0/esptool.py --chip esp32 --port \"{serial.port}\" --baud 921600  --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 \"{build.path}/{build.project_name}.bootloader.bin\" 0x8000 \"{build.path}/{build.project_name}.partitions.bin\" 0xe000 /home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/tools/partitions/boot_app0.bin 0x10000 \"{build.path}/{build.project_name}.bin\" "
)
set(ARDUINO_TOOL_ALTARGS
	"."
	""
)
set(ARDUINO_TOOL "esptool_py")
set(ARDUINO_TOOL_ACTION "upload")
set(ARDUINO_RESOLVE_TARGET "TRUE")

set(options_usage "")
set(missing_options "")
set(all_options "")
set(_script "${CMAKE_CURRENT_LIST_FILE}")
get_filename_component(_action_str "${_script}" NAME_WE)

# Format the string for displaying help for usage
macro(_append_usage_for_option option default_var)

	# Concat all the parts of the help string
	set(_help_str "")
	foreach(_arg IN ITEMS ${ARGN})
		set(_help_str "${_help_str}${_arg}")
	endforeach()
	# Append default value
	if (NOT "${default_var}" STREQUAL "")
		set(_help_str "${_help_str} (Default: '${${default_var}}')")
	endif()
	# Append the option name as header followed by required space length
	set(_usage_str "  ${option}")
	string(LENGTH "${_usage_str}" _usage_str_len)
	if (NOT _usage_str_len LESS 30)
		set(_usage_str "${_usage_str}\n")
		set(_usage_str_len 0)
	endif()
	# Append the help string split into shorter strings at a fixed position
	while(NOT "${_help_str}" STREQUAL "")
		# Add space
		foreach(i RANGE ${_usage_str_len} 30)
			set(_usage_str "${_usage_str} ") # Add gap
		endforeach()
		# Split _help_str not more than 40 characters
		string(SUBSTRING "${_help_str}" 0 40 _str_part)
		if (NOT _str_part STREQUAL _help_str)
			string(REGEX REPLACE "\r?\n.*$" "" _str_part "${_str_part}")
			string(REGEX REPLACE "[ \t][^ \t]*$" "" _str_part "${_str_part}")
		endif()
		string(LENGTH "${_str_part}" _str_len)
		set(_usage_str "${_usage_str}${_str_part}\n")
		string(SUBSTRING "${_help_str}" "${_str_len}" -1 _help_str)
		string(STRIP "${_help_str}" _help_str)
		set(_usage_str_len 0)
	endwhile()
	# Append the formatted usage
	set(options_usage "${options_usage}${_usage_str}")
	set(options_usage "${options_usage}" PARENT_SCOPE)

endmacro()

# Read an entry from cache
function(_read_cache_variable var_regex return_var return_value)

	set(_var "")
	set(_value "")
	file(STRINGS "${ARDUINO_BINARY_DIR}/CMakeCache.txt" _var_str_list
		REGEX "^${var_regex}:[^=]+=.*$")
	foreach(_var_str IN LISTS _var_str_list)
		string(REGEX MATCH "^(${var_regex}):[^=]+=(.*)$" _var_str "${_var_str}")
		list(APPEND _var "${CMAKE_MATCH_1}")
		list(APPEND _value "${CMAKE_MATCH_2}")
	endforeach()
	set("${return_var}" "${_var}" PARENT_SCOPE)
	set("${return_value}" "${_value}" PARENT_SCOPE)

endfunction()

# Build and resolve the target passed through environment variable
function(_build_and_resolve_target _string return_string)

	# If the target properties are already passed to this script, nothing
	# to do further here, and we assume that any build dependency is already
	# taken care
	if (DEFINED ARDUINO_BUILD_PATH)
		string(REPLACE "{build.project_name}" "${ARDUINO_BUILD_PROJECT_NAME}"
			_string "${_string}")
		string(REPLACE "{build.path}" "${ARDUINO_BUILD_PATH}"
			_string "${_string}")
		string(REPLACE "{build.source.path}" "${ARDUINO_BUILD_SOURCE_PATH}"
			_string "${_string}")
		set("${return_string}" "${_string}" PARENT_SCOPE)
		return()
	endif()

	# Append usage string
	list(APPEND option_values_ "TARGET")
	set(option_values_ "${option_values_}" PARENT_SCOPE)
	
	file(GLOB _app_script_list
		"${ARDUINO_BINARY_DIR}/.app_targets/*.cmake")
	set(_app_list)
	foreach(_app_script IN LISTS _app_script_list)
		get_filename_component(_app_name "${_app_script}" NAME)
		string(REGEX MATCH "^(.*)\\.cmake$" _match "${_app_name}")
		list(APPEND _app_list "${CMAKE_MATCH_1}")
		list(APPEND option_values_TARGET "${CMAKE_MATCH_1}")
	endforeach()
	set(option_values_TARGET "${option_values_TARGET}" PARENT_SCOPE)
	unset(_target)
	list(LENGTH _app_list _num_apps)
	if (_num_apps EQUAL 1)
		set(_def_app_var _app_list)
		set(_target "${_app_list}")
	else()
		set(_def_app_var "")
	endif()
	if (DEFINED ENV{TARGET})
		set(_target "$ENV{TARGET}")
	endif()
	string(REPLACE ";" ", " _app_list "${_app_list}")
	_append_usage_for_option("TARGET" "${_def_app_var}"
		"Target for ${_action_str}. Possible targets are ${_app_list}.")
	if (NOT DEFINED _target OR _script_dry_run)
		list(APPEND missing_options "TARGET")
		set(missing_options "${missing_options}" PARENT_SCOPE)
		string(REPLACE "{build.project_name}" "" _string "${_string}")
		string(REPLACE "{build.path}" "" _string "${_string}")
		string(REPLACE "{build.source.path}" "" _string "${_string}")
		set("${return_string}" "${_string}" PARENT_SCOPE)
		return()
	endif()

	# Build the target
	execute_process(COMMAND ${CMAKE_COMMAND} --build
		"${ARDUINO_BINARY_DIR}" --target "${_target}"
		RESULT_VARIABLE _build_result)
	if (NOT _build_result EQUAL 0)
		message(FATAL_ERROR
			"*** ${_action_str} for '${_target}' failed!")
	endif()

	# Resolve the target source and binary directories
	include("${ARDUINO_BINARY_DIR}/.app_targets/${_target}.cmake")
	string(REPLACE "{build.project_name}" "${ARDUINO_BUILD_PROJECT_NAME}"
		_string "${_string}")
	string(REPLACE "{build.path}" "${ARDUINO_BUILD_PATH}"
		_string "${_string}")
	string(REPLACE "{build.source.path}" "${ARDUINO_BUILD_SOURCE_PATH}"
		_string "${_string}")
	set("${return_string}" "${_string}" PARENT_SCOPE)

endfunction()

# Resolve alternate arguments
function(_resolve_alt_args _string return_string)

	math(EXPR _alt_arg_idx "${tool_idx}+1")
	list(GET ARDUINO_TOOL_ALTARGS "${_alt_arg_idx}" _alt_args)
	separate_arguments(_alt_args UNIX_COMMAND "${_alt_args}")
	list(LENGTH _alt_args _alt_args_len)
	set(_idx 0)
	while (_idx LESS _alt_args_len)
		list(GET _alt_args "${_idx}" _alt_arg)
		math(EXPR _idx "${_idx} + 1")
		list(GET _alt_args "${_idx}" _alt_arg_true)
		math(EXPR _idx "${_idx} + 1")
		list(GET _alt_args "${_idx}" _alt_arg_false)
		math(EXPR _idx "${_idx} + 1")

		set(_var_name "${ARDUINO_TOOL_ACTION}.${_alt_arg}")
		string(TOUPPER "${_var_name}" _cache_var)
		string(MAKE_C_IDENTIFIER "${_cache_var}" _cache_var)
		if (NOT "${_alt_arg}" STREQUAL "verbose")
			string(TOUPPER "${_alt_arg}" _env_var)
		else()
			set(_env_var "${_cache_var}")
		endif()

		# Append usage string
		_read_cache_variable("ARDUINO_${_cache_var}" _dummy
	        _def_val)
		_append_usage_for_option("${_env_var}" "_def_val"
			"Enable ${_alt_arg} mode for ${ARDUINO_TOOL_ACTION}.")

		# Add to the list of options and option values
		list(APPEND option_values_ "${_env_var}")
		set(option_values_${_env_var} "TRUE;FALSE" PARENT_SCOPE)

		# Resolve the arguments
		if ((NOT DEFINED ENV{${_env_var}} AND "${_def_val}") OR
			(DEFINED ENV{${_env_var}} AND "$ENV{${_env_var}}"))
			set(_args "${_alt_arg_true}")
		else()
			set(_args "${_alt_arg_false}")
		endif()
		string(REPLACE "{${_var_name}}" "${_args}" _string "${_string}")

	endwhile()

	set(option_values_ "${option_values_}" PARENT_SCOPE)
	set("${return_string}" "${_string}" PARENT_SCOPE)

endfunction()

# Resolve variables
function(_resolve_variables _string return_string)

	string(REGEX MATCHALL "{[^{}/]+}" _var_list "${_string}")
	if (NOT "${_var_list}" STREQUAL "")
		list(REMOVE_DUPLICATES _var_list)
	endif()
	foreach(_var_str IN LISTS _var_list)
		string(REGEX MATCH "{(${ARDUINO_TOOL_ACTION}\\.)?(.+)}"
			_match "${_var_str}")
		set(_var_name "${CMAKE_MATCH_2}")
		string(MAKE_C_IDENTIFIER "${_var_name}" _env_var)
		string(TOUPPER "${_env_var}" _env_var)
		string(TOUPPER "${ARDUINO_TOOL_ACTION}_${_env_var}"
			_cache_var)

		# Add to the list of options and option values
		list(APPEND option_values_ "${_env_var}")
		set(option_values_${_env_var} "" PARENT_SCOPE)

		# Append usage string
		set(_def_val_var "")
		_read_cache_variable("ARDUINO_${_cache_var}" _def_val_var
	        _def_val)
		if (_def_val_var)
			set(_def_val_var "_def_val")
		endif()
		string(REPLACE "." " " _var_name_help "${_var_name}")
		_append_usage_for_option("${_env_var}" "${_def_val_var}"
			"Value for ${_var_name_help}.")
		
		# Resolve the value
		if (DEFINED ENV{${_env_var}})
			string(REPLACE "${_var_str}" "$ENV{${_env_var}}"
				_string "${_string}")
		elseif(NOT "${_def_val_var}" STREQUAL "")
			string(REPLACE "${_var_str}" "${_def_val}"
				_string "${_string}")
		else()
			list(APPEND missing_options "${_env_var}")
			set(missing_options "${missing_options}" PARENT_SCOPE)
		endif()
	endforeach()

	set(option_values_ "${option_values_}" PARENT_SCOPE)
	set("${return_string}" "${_string}" PARENT_SCOPE)

endfunction()

# Resolve tool
function(_resolve_tool return_idx return_cmdline)
	string(TOUPPER "${ARDUINO_TOOL_ACTION}" ARDUINO_TOOL_ACTION_VAR)
	list(LENGTH ARDUINO_TOOL_NAMES _num_patterns)
	list(APPEND option_values_ "TOOL")
	set(option_values_ "${option_values_}" PARENT_SCOPE)
	if (_num_patterns GREATER 1)
		string(REPLACE ";" ", " _tool_names "${ARDUINO_TOOL_NAMES}")
		set(_def_tool_var "")
		if (NOT "${ARDUINO_TOOL}" STREQUAL "")
			set(_def_tool_var "ARDUINO_TOOL")
		endif()
		_append_usage_for_option("TOOL" "${_def_tool_var}"
			"Tool for ${_action_str}. "
			"Possible tools are ${_tool_names}.")
	endif()
	set(option_values_TOOL "${ARDUINO_TOOL_NAMES}" PARENT_SCOPE)
	set(option_values_TOOL_DESCRIPTIONS "${ARDUINO_TOOL_DESCRIPTIONS}" PARENT_SCOPE)
	if (DEFINED ENV{TOOL})
		set(_sel_tool "$ENV{TOOL}")
	else()
		set(_sel_tool "${ARDUINO_TOOL}")
	endif()
	set(_idx 0)
	set(_tool "")
	while("${_idx}" LESS "${_num_patterns}")
		list(GET ARDUINO_TOOL_NAMES "${_idx}" _tool_name)
		list(GET ARDUINO_TOOL_CMDLINE_LIST "${_idx}" _tool_cmdline)
		if ("${_tool_name}" STREQUAL "${_sel_tool}")
			set(_tool "${_tool_name}")
			break()
		endif()
		math(EXPR _idx "${_idx} + 1")
	endwhile()

	if (NOT _tool)
		list(APPEND missing_options "TOOL")
		set(missing_options "${missing_options}" PARENT_SCOPE)
		set("${return_idx}" "-1" PARENT_SCOPE)
		set("${return_cmdline}" "" PARENT_SCOPE)
		return()
	endif()
	set("${return_idx}" "${_idx}" PARENT_SCOPE)
	set("${return_cmdline}" "${_tool_cmdline}" PARENT_SCOPE)
endfunction()

# This script can be used to list the possible options or the values of a
# particular option without actually executing the tool.
set(_script_dry_run FALSE)
if (DEFINED ENV{ARDUINO_LIST_OPTION_VALUES})
	set(_script_dry_run TRUE)
endif()

# Resolve the variables
_resolve_tool(tool_idx tool_cmdline)
if (ARDUINO_RESOLVE_TARGET OR DEFINED ENV{TARGET})
	_build_and_resolve_target("${tool_cmdline}" tool_cmdline)
endif()
_resolve_alt_args("${tool_cmdline}" tool_cmdline)
_resolve_variables("${tool_cmdline}" tool_cmdline)

# Perform the required function
if (DEFINED ENV{ARDUINO_LIST_OPTION_VALUES})
	# Request to list the options or option values
	string(REPLACE ";" "\n" _output_str
		"${option_values_$ENV{ARDUINO_LIST_OPTION_VALUES}}")
	execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${_output_str}")
elseif (NOT missing_options STREQUAL "")
	# Report usage error
	string(REPLACE ";" ", " _options "${missing_options}")
	set(_usage
		"\n*** Required environment variables missing: ${_options}\n")
	if (DEFINED MAKE_TARGET)
		_read_cache_variable(CMAKE_MAKE_PROGRAM _var _make_program)
		if (_var)
			get_filename_component(_make_program "${_make_program}" NAME_WE)
		else()
			set(_make_program "<make>")
		endif()
		set(_usage
			"${_usage}Usage: ${_make_program} ${MAKE_TARGET}")
		set(_usage
			"${_usage} [<option>=<value>]...\n")
	else()
		get_filename_component(_cmake_program "${CMAKE_COMMAND}" NAME_WE)
		get_filename_component(_script_name "${_script}" NAME)
		set(_usage
			"${_usage}Usage: ${_cmake_program} -E env [<option>=<value>]...")
		set(_usage
			"${_usage} ${_cmake_program} -P .scripts/${_script_name}\n")
	endif()
	set(_usage
		"${_usage}where <options> are\n")
	set(_usage "${_usage}${options_usage}")
	execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${_usage}")
	message(FATAL_ERROR "*** ${_action_str} failed!")
else()
	# execute the tool
	separate_arguments(_pattern UNIX_COMMAND "${tool_cmdline}")
	if (CMAKE_VERBOSE_MAKEFILE OR DEFINED ENV{VERBOSE})
		execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${tool_cmdline}")
	endif()
	
	execute_process(COMMAND ${_pattern} RESULT_VARIABLE result)
	if (NOT "${result}" EQUAL 0)
		message(FATAL_ERROR "*** ${_action_str} failed!")
	endif()
endif()

