# Copyright (c) 2020 Arduino CMake Toolchain

cmake_policy(VERSION 3.0)

# Generated content based on the selected board
set(ARDUINO_TOOLCHAIN_DIR "/home/neumann/W/Tria/rinasense/Arduino-CMake-Toolchain")
set(ARDUINO_LINK_PATTERN " \"-Wl,--Map={build.path}/{build.project_name}.map\" -L/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/tools/sdk/esp32/lib -L/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/tools/sdk/esp32/ld -L/home/neumann/.arduino15/packages/esp32/hardware/esp32/2.0.4/tools/sdk/esp32/dio_qspi -T esp32.rom.redefined.ld -T memory.ld -T sections.ld -T esp32.rom.ld -T esp32.rom.api.ld -T esp32.rom.libgcc.ld -T esp32.rom.newlib-data.ld -T esp32.rom.syscalls.ld -T esp32.peripherals.ld  -mlongcalls -Wno-frame-address -Wl,--cref -Wl,--gc-sections -fno-rtti -fno-lto -u ld_include_hli_vectors_bt -u _Z5setupv -u _Z4loopv -u esp_app_desc -u pthread_include_pthread_impl -u pthread_include_pthread_cond_impl -u pthread_include_pthread_local_storage_impl -u pthread_include_pthread_rwlock_impl -u include_esp_phy_override -u ld_include_highint_hdl -u start_app -u start_app_other_cores -u __ubsan_include -Wl,--wrap=longjmp -u __assert_func -u vfs_include_syscalls_impl -Wl,--undefined=uxTopUsedPriority -u app_main -u newlib_include_heap_impl -u newlib_include_syscalls_impl -u newlib_include_pthread_impl -u newlib_include_assert_impl -u __cxa_guard_dummy  -DESP32 -DCORE_DEBUG_LEVEL=0    -DARDUINO_USB_CDC_ON_BOOT=0 -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> {archive_file_path}  -lesp_ringbuf -lefuse -lesp_ipc -ldriver -lesp_pm -lmbedtls -lapp_update -lbootloader_support -lspi_flash -lnvs_flash -lpthread -lesp_gdbstub -lespcoredump -lesp_phy -lesp_system -lesp_rom -lhal -lvfs -lesp_eth -ltcpip_adapter -lesp_netif -lesp_event -lwpa_supplicant -lesp_wifi -lconsole -llwip -llog -lheap -lsoc -lesp_hw_support -lxtensa -lesp_common -lesp_timer -lfreertos -lnewlib -lcxx -lapp_trace -lasio -lbt -lcbor -lunity -lcmock -lcoap -lnghttp -lesp-tls -lesp_adc_cal -lesp_hid -ltcp_transport -lesp_http_client -lesp_http_server -lesp_https_ota -lesp_https_server -lesp_lcd -lprotobuf-c -lprotocomm -lmdns -lesp_local_ctrl -lsdmmc -lesp_serial_slave_link -lesp_websocket_client -lexpat -lwear_levelling -lfatfs -lfreemodbus -ljsmn -ljson -llibsodium -lmqtt -lopenssl -lperfmon -lspiffs -lulp -lwifi_provisioning -lbutton -lrmaker_common -ljson_parser -ljson_generator -lesp_schedule -lesp_rainmaker -lqrcode -lws2812_led -lesp-dsp -lesp-sr -lesp32-camera -lesp_littlefs -lfb_gfx -lasio -lcbor -lcmock -lunity -lcoap -lesp_lcd -lesp_websocket_client -lexpat -lfreemodbus -ljsmn -llibsodium -lperfmon -lesp_adc_cal -lesp_hid -lfatfs -lwear_levelling -lopenssl -lesp_rainmaker -lesp_local_ctrl -lesp_https_server -lwifi_provisioning -lprotocomm -lbt -lbtdm_app -lprotobuf-c -lmdns -lrmaker_common -lmqtt -ljson_parser -ljson_generator -lesp_schedule -lqrcode -lcat_face_detect -lhuman_face_detect -lcolor_detect -lmfn -ldl -lwakenet -lmultinet -lesp_audio_processor -lesp_audio_front_end -lesp-sr -lwakenet -lmultinet -lesp_audio_processor -lesp_audio_front_end -ljson -lspiffs -ldl_lib -lc_speech_features -lhilexin_wn5 -lhilexin_wn5X2 -lhilexin_wn5X3 -lnihaoxiaozhi_wn5 -lnihaoxiaozhi_wn5X2 -lnihaoxiaozhi_wn5X3 -lnihaoxiaoxin_wn5X3 -lcustomized_word_wn5 -lmultinet2_ch -lesp_tts_chinese -lvoice_set_xiaole -lesp_ringbuf -lefuse -lesp_ipc -ldriver -lesp_pm -lmbedtls -lapp_update -lbootloader_support -lspi_flash -lnvs_flash -lpthread -lesp_gdbstub -lespcoredump -lesp_phy -lesp_system -lesp_rom -lhal -lvfs -lesp_eth -ltcpip_adapter -lesp_netif -lesp_event -lwpa_supplicant -lesp_wifi -lconsole -llwip -llog -lheap -lsoc -lesp_hw_support -lxtensa -lesp_common -lesp_timer -lfreertos -lnewlib -lcxx -lapp_trace -lnghttp -lesp-tls -ltcp_transport -lesp_http_client -lesp_http_server -lesp_https_ota -lsdmmc -lesp_serial_slave_link -lulp -lmbedtls_2 -lmbedcrypto -lmbedx509 -lcoexist -lcore -lespnow -lmesh -lnet80211 -lpp -lsmartconfig -lwapi -lesp_ringbuf -lefuse -lesp_ipc -ldriver -lesp_pm -lmbedtls -lapp_update -lbootloader_support -lspi_flash -lnvs_flash -lpthread -lesp_gdbstub -lespcoredump -lesp_phy -lesp_system -lesp_rom -lhal -lvfs -lesp_eth -ltcpip_adapter -lesp_netif -lesp_event -lwpa_supplicant -lesp_wifi -lconsole -llwip -llog -lheap -lsoc -lesp_hw_support -lxtensa -lesp_common -lesp_timer -lfreertos -lnewlib -lcxx -lapp_trace -lnghttp -lesp-tls -ltcp_transport -lesp_http_client -lesp_http_server -lesp_https_ota -lsdmmc -lesp_serial_slave_link -lulp -lmbedtls_2 -lmbedcrypto -lmbedx509 -lcoexist -lcore -lespnow -lmesh -lnet80211 -lpp -lsmartconfig -lwapi -lesp_ringbuf -lefuse -lesp_ipc -ldriver -lesp_pm -lmbedtls -lapp_update -lbootloader_support -lspi_flash -lnvs_flash -lpthread -lesp_gdbstub -lespcoredump -lesp_phy -lesp_system -lesp_rom -lhal -lvfs -lesp_eth -ltcpip_adapter -lesp_netif -lesp_event -lwpa_supplicant -lesp_wifi -lconsole -llwip -llog -lheap -lsoc -lesp_hw_support -lxtensa -lesp_common -lesp_timer -lfreertos -lnewlib -lcxx -lapp_trace -lnghttp -lesp-tls -ltcp_transport -lesp_http_client -lesp_http_server -lesp_https_ota -lsdmmc -lesp_serial_slave_link -lulp -lmbedtls_2 -lmbedcrypto -lmbedx509 -lcoexist -lcore -lespnow -lmesh -lnet80211 -lpp -lsmartconfig -lwapi -lesp_ringbuf -lefuse -lesp_ipc -ldriver -lesp_pm -lmbedtls -lapp_update -lbootloader_support -lspi_flash -lnvs_flash -lpthread -lesp_gdbstub -lespcoredump -lesp_phy -lesp_system -lesp_rom -lhal -lvfs -lesp_eth -ltcpip_adapter -lesp_netif -lesp_event -lwpa_supplicant -lesp_wifi -lconsole -llwip -llog -lheap -lsoc -lesp_hw_support -lxtensa -lesp_common -lesp_timer -lfreertos -lnewlib -lcxx -lapp_trace -lnghttp -lesp-tls -ltcp_transport -lesp_http_client -lesp_http_server -lesp_https_ota -lsdmmc -lesp_serial_slave_link -lulp -lmbedtls_2 -lmbedcrypto -lmbedx509 -lcoexist -lcore -lespnow -lmesh -lnet80211 -lpp -lsmartconfig -lwapi -lesp_ringbuf -lefuse -lesp_ipc -ldriver -lesp_pm -lmbedtls -lapp_update -lbootloader_support -lspi_flash -lnvs_flash -lpthread -lesp_gdbstub -lespcoredump -lesp_phy -lesp_system -lesp_rom -lhal -lvfs -lesp_eth -ltcpip_adapter -lesp_netif -lesp_event -lwpa_supplicant -lesp_wifi -lconsole -llwip -llog -lheap -lsoc -lesp_hw_support -lxtensa -lesp_common -lesp_timer -lfreertos -lnewlib -lcxx -lapp_trace -lnghttp -lesp-tls -ltcp_transport -lesp_http_client -lesp_http_server -lesp_https_ota -lsdmmc -lesp_serial_slave_link -lulp -lmbedtls_2 -lmbedcrypto -lmbedx509 -lcoexist -lcore -lespnow -lmesh -lnet80211 -lpp -lsmartconfig -lwapi -lphy -lrtc -lesp_phy -lphy -lrtc -lesp_phy -lphy -lrtc -lxt_hal -lm -lnewlib -lstdc++ -lpthread -lgcc -lcxx -lapp_trace -lgcov -lapp_trace -lgcov -lc  -Wl,--end-group -Wl,-EL -o <TARGET>")
set(ARDUINO_GENERATED_FILES "{build.path}/partitions.csv;{build.path}/partitions.csv;{build.path}/partitions.csv;{build.path}/{build.project_name}.bootloader.bin;{build.path}/build_opt.h;{build.path}/build_opt.h")
set(ARDUINO_CORE_OBJECT_FILES "core/esp32-hal-adc.c.o;core/esp32-hal-bt.c.o;core/esp32-hal-cpu.c.o;core/esp32-hal-dac.c.o;core/esp32-hal-gpio.c.o;core/esp32-hal-i2c-slave.c.o;core/esp32-hal-i2c.c.o;core/esp32-hal-ledc.c.o;core/esp32-hal-matrix.c.o;core/esp32-hal-misc.c.o;core/esp32-hal-psram.c.o;core/esp32-hal-rgb-led.c.o;core/esp32-hal-rmt.c.o;core/esp32-hal-sigmadelta.c.o;core/esp32-hal-spi.c.o;core/esp32-hal-time.c.o;core/esp32-hal-timer.c.o;core/esp32-hal-tinyusb.c.o;core/esp32-hal-touch.c.o;core/esp32-hal-uart.c.o;core/firmware_msc_fat.c.o;core/libb64/cdecode.c.o;core/libb64/cencode.c.o;core/stdlib_noniso.c.o;core/wiring_pulse.c.o;core/wiring_shift.c.o;core/Esp.cpp.o;core/FirmwareMSC.cpp.o;core/FunctionalInterrupt.cpp.o;core/HWCDC.cpp.o;core/HardwareSerial.cpp.o;core/IPAddress.cpp.o;core/IPv6Address.cpp.o;core/MD5Builder.cpp.o;core/Print.cpp.o;core/Stream.cpp.o;core/StreamString.cpp.o;core/Tone.cpp.o;core/USB.cpp.o;core/USBCDC.cpp.o;core/USBMSC.cpp.o;core/WMath.cpp.o;core/WString.cpp.o;core/base64.cpp.o;core/cbuf.cpp.o;core/main.cpp.o")
if (NOT DEFINED ARDUINO_BINARY_DIR)
	set(ARDUINO_BINARY_DIR "/home/neumann/W/Tria/rinasense/Apps/arduino-basic")
	set(ARDUINO_PROJECT_DIR "/home/neumann/W/Tria/rinasense")
	set(ARDUINO_PROJECT_NAME "RINA_sensor")
endif()

include("${ARDUINO_TOOLCHAIN_DIR}/Arduino/Utilities/CommonUtils.cmake")

if ("${ARG_TARGET_NAME}" STREQUAL "")
	message(FATAL_ERROR "Script not allowed to be invoked directly!!!")
endif()

# Read an entry from cache
function(_read_cache_variable var_regex return_var return_value)

	set(_var "")
	set(_value "")
	file(STRINGS "${ARDUINO_BINARY_DIR}/CMakeCache.txt" _var_str_list
		REGEX "^(${var_regex}):[^=]+=(.*)$")
	foreach(_var_str IN LISTS _var_str_list)
		string(REGEX MATCH "^(${var_regex}):[^=]+=(.*)$" _var_str "${_var_str}")
		list(APPEND _var "${CMAKE_MATCH_1}")
		list(APPEND _value "${CMAKE_MATCH_2}")
	endforeach()
	set("${return_var}" "${_var}" PARENT_SCOPE)
	set("${return_value}" "${_value}" PARENT_SCOPE)

endfunction()

# Resolve target information
function(_resolve_target_info _str return_str)
	string(REPLACE "{build.path}" "${ARDUINO_BUILD_PATH}" _str "${_str}")
	string(REPLACE "{build.project_name}" "${ARDUINO_BUILD_PROJECT_NAME}"
		_str "${_str}")
	string(REPLACE "{build.source.path}" "${ARDUINO_BUILD_SOURCE_PATH}"
		_str "${_str}")
	set("${return_str}" "${_str}" PARENT_SCOPE)
endfunction()

function(_find_target_project_source_path binary_path return_path)

	# Find the nearest project directory for the given binary directory
	_read_cache_variable("[^:]+_BINARY_DIR" _var_list _value_list)
	set(_idx 0)
	set(max_len 0)
	list(LENGTH _var_list _num_vars)
	while(_idx LESS _num_vars)
		list(GET _var_list ${_idx} _var_name)
		list(GET _value_list ${_idx} _value)
		math(EXPR _idx "${_idx}+1")
		string(LENGTH "${_value}" path_len)
		string(SUBSTRING binary_path 0 ${path_len} _build_path)
		if (_build_path STREQUAL _value AND max_len LESS path_len)
			set(best_var "${_var_name}")
		endif()
	endwhile()
	string(REGEX MATCH "^(.*)_BINARY_DIR$" _match "${best_var}")
	if (_match)
		_read_cache_variable("${CMAKE_MATCH_1}_SOURCE_DIR" _var
			_source_path)
	else()
		set(_source_path "${ARDUINO_PROJECT_DIR}")
	endif()
	set("${return_path}" "${_source_path}" PARENT_SCOPE)

endfunction()

# Search for core library
set(_core_lib "")
set(_link_libs "")
#message("ARG_LINK_LIBRARIES:${ARG_LINK_LIBRARIES}")
string(REPLACE "\\" "\\\\" ARG_LINK_LIBRARIES "${ARG_LINK_LIBRARIES}")
separate_arguments(ARG_LINK_LIBRARIES UNIX_COMMAND "${ARG_LINK_LIBRARIES}")
#message("ARG_OBJECTS:${ARG_OBJECTS}")
string(REPLACE "\\" "\\\\" ARG_OBJECTS "${ARG_OBJECTS}")
separate_arguments(ARG_OBJECTS UNIX_COMMAND "${ARG_OBJECTS}")
#message("ARG_LINK_FLAGS:${ARG_LINK_FLAGS}")
string(REPLACE "\\" "\\\\" ARG_LINK_FLAGS "${ARG_LINK_FLAGS}")
separate_arguments(ARG_LINK_FLAGS UNIX_COMMAND "${ARG_LINK_FLAGS}")
#message("ARDUINO_LINK_PATTERN:${ARDUINO_LINK_PATTERN}")
string(REPLACE "\\" "\\\\" ARDUINO_LINK_PATTERN "${ARDUINO_LINK_PATTERN}")
separate_arguments(_link_pattern UNIX_COMMAND "${ARDUINO_LINK_PATTERN}")

foreach(lib IN LISTS ARG_LINK_LIBRARIES)
	if (EXISTS "${lib}.ard_core_info")
		if (NOT _core_lib)
			set(_core_lib "${lib}")
			include("${lib}.ard_core_info")
		endif()
	else()
		list(APPEND _link_libs "${lib}")
	endif()
endforeach()

# Replace {archive_file_path} with found library
if (_core_lib)
	if (NOT "${ARDUINO_VARIANT_OBJECTS}" STREQUAL "")
		list(APPEND ARG_OBJECTS ${ARDUINO_VARIANT_OBJECTS})
	endif()
	string(REPLACE "{archive_file_path}" "${_core_lib}" _link_pattern
		"${_link_pattern}")
else()
	string(REPLACE "\"{archive_file_path}\"|{archive_file_path}" ""
		_link_pattern "${_link_pattern}")
endif()

# Target information from cache
set(ARDUINO_BUILD_PROJECT_NAME "${ARG_TARGET_NAME}")
if (EXISTS "${ARDUINO_BINARY_DIR}/.app_targets/${ARG_TARGET_NAME}.cmake")
	include("${ARDUINO_BINARY_DIR}/.app_targets/${ARG_TARGET_NAME}.cmake")
else()
	# Find the nearest project directory for the target
	_find_target_project_source_path("${ARDUINO_BUILD_PATH}"
		ARDUINO_BUILD_SOURCE_PATH)
endif()
set(ARDUINO_BUILD_PATH "${CMAKE_BINARY_DIR}")

# Find all {build.path} search/replace list
set(_search_list "")
set(_replace_list "")

# Find {build.path}/... with generated files found in the current directory
foreach(_gen_file IN LISTS ARDUINO_GENERATED_FILES)
	list(APPEND _search_list "${_gen_file}")
	# Resolve target info, and if generated file is found, use it
	_resolve_target_info("${_gen_file}" _match_file)
	if (EXISTS "${_match_file}")
		list(APPEND _replace_list "${_match_file}")
		continue()
	endif()
	# Resolve project info, and if generated file is found, use it
	string(REPLACE "{build.path}" "${ARDUINO_BINARY_DIR}" _proj_file
		"${_gen_file}")
	string(REPLACE "{build.project_name}" "${ARDUINO_PROJECT_NAME}"
		_proj_file "${_proj_file}")
	string(REPLACE "{build.source.path}" "${ARDUINO_PROJECT_DIR}"
		_proj_file "${_proj_file}")
	if (EXISTS "${_proj_file}")
		list(APPEND _replace_list "${_proj_file}")
		continue()
	endif()
	# As a fallback, use target resolved path
	list(APPEND _replace_list "${_match_file}")
endforeach()

# Find {build.path}/<core_obj_file> with object files from the core
function(_find_obj_file_match obj_file core_obj_files return_match_file)
	string(REGEX MATCH "^([^/]+)/(.+)$" _match "${obj_file}")
	set(_obj_file_type "${CMAKE_MATCH_1}")
	set(_obj_file_regex "${CMAKE_MATCH_2}")
	string_escape_regex(_obj_file_regex "${_obj_file_regex}")
	while (TRUE)
		set(_core_obj_match_files "${core_obj_files}")
		list_filter_include_regex(_core_obj_match_files  "${_obj_file_regex}$")
		list(LENGTH _core_obj_match_files _match_len)
		if (_match_len EQUAL 0)
			# Try to look for sub folder match
			string(REGEX MATCH "^([^/]+)/(.+)$" _match "${_obj_file_regex}")
			if ("${_match}" STREQUAL "")
				# No such object file found!
				set("${return_match_file}" "{build.path}/${obj_file}")
				break()
			endif()
			# Try again with new regex
			set(_obj_file_regex "${CMAKE_MATCH_2}")
		elseif (_match_len EQUAL 1)
			set("${return_match_file}" "${_core_obj_match_files}")
			break()
		else()
			# Too many matches, pick the first one?
			list(GET _core_obj_match_files 0 _match_file)
			set("${return_match_file}" "${_match_file}")
			break()
		endif()
	endwhile()
	# message("${obj_file} => ${${return_match_file}}")
	set("${return_match_file}" "${${return_match_file}}" PARENT_SCOPE)
endfunction()

foreach(_obj_file IN LISTS ARDUINO_CORE_OBJECT_FILES)
	if (_obj_file MATCHES "^core/.*")
		_find_obj_file_match("${_obj_file}" "${ARDUINO_CORE_OBJECTS}"
			_match_obj_file)
		list(APPEND _search_list "{build.path}/${_obj_file}")
		list(APPEND _replace_list "${_match_obj_file}")
	elseif(_obj_file MATCHES "^variant/.*")
		_find_obj_file_match("${_obj_file}" "${ARDUINO_VARIANT_OBJECTS}"
			_match_obj_file)
		list(APPEND _search_list "{build.path}/${_obj_file}")
		list(APPEND _replace_list "${_match_obj_file}")
	endif()
endforeach()

list(APPEND _search_list "{build.path}")
list(APPEND _replace_list "${ARDUINO_BUILD_PATH}")

# Search/replace {build.path}
set(_link_pattern_part "${_link_pattern}")
set(_link_pattern "")
while(TRUE)
	# message("Part:${_link_pattern_part}")
	string(FIND "${_link_pattern_part}" "{build.path}" _idx)
	string(SUBSTRING "${_link_pattern_part}" 0 "${_idx}" _prefix)
	# message("Prefix:${_prefix}")
	set(_link_pattern "${_link_pattern}${_prefix}")
	if (_idx LESS 0)
		break()
	endif()
	string(SUBSTRING "${_link_pattern_part}" "${_idx}" -1 _link_pattern_part)
	# message("RemainPart:${_link_pattern_part}")
	set(_list_idx 0)
	list(LENGTH _search_list _list_len)
	while(_list_idx LESS _list_len)
		list(GET _search_list ${_list_idx} _search_elem)
		list(GET _replace_list ${_list_idx} _replace_elem)
		math(EXPR _list_idx "${_list_idx} + 1")
		string(LENGTH "${_search_elem}" _str_len)
		string(SUBSTRING "${_link_pattern_part}" 0 "${_str_len}" _mtch_part)
		if ("${_search_elem}" STREQUAL "${_mtch_part}")
			set(_link_pattern "${_link_pattern}${_replace_elem}")
			string(SUBSTRING "${_link_pattern_part}" "${_str_len}" -1
				_link_pattern_part)
			break()
		endif()
	endwhile()
endwhile()
# message("${_link_pattern}")

# All other search/replace
set(_semicolon ";")
set(_angle_r ">")

set(_search_list
	"<SEMICOLON>"
	"<TARGET>"
	"{build.source.path}"
	"{build.project_name}"
	"<LINK_FLAGS>"
	"<OBJECTS>"
	"<LINK_LIBRARIES>"
	"<ANGLE-R>")

set(_replace_list
	_semicolon
	ARG_TARGET
	ARDUINO_BUILD_SOURCE_PATH
	ARDUINO_BUILD_PROJECT_NAME
	ARG_LINK_FLAGS
	ARG_OBJECTS
	_link_libs
	_angle_r)

list(LENGTH _search_list _list_len)
set(_list_idx 0)
while(_list_idx LESS _list_len)
	list(GET _search_list ${_list_idx} _search_elem)
	list(GET _replace_list ${_list_idx} _replace_var)
	math(EXPR _list_idx "${_list_idx} + 1")
	string(REPLACE "${_search_elem}" "${${_replace_var}}" _link_pattern
		"${_link_pattern}")
endwhile()

if (CMAKE_VERBOSE_MAKEFILE OR DEFINED ENV{VERBOSE})
	set(printable_cmd_line "\"${ARDUINO_LINK_COMMAND}\"")
	foreach(_arg IN LISTS _link_pattern)
		if (_arg MATCHES "[ \t\\\\|&<>;*?]")
			set(printable_cmd_line "${printable_cmd_line} \"${_arg}\"")
		else()
			set(printable_cmd_line "${printable_cmd_line} ${_arg}")
		endif()
	endforeach()
	message("${printable_cmd_line}")
endif()

execute_process(COMMAND "${ARDUINO_LINK_COMMAND}" ${_link_pattern} RESULT_VARIABLE result)
if (NOT "${result}" EQUAL 0)
	message(FATAL_ERROR "Linking ${ARG_TARGET_NAME} failed!!!")
endif()
