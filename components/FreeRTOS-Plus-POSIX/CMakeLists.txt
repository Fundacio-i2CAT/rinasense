if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  file(TO_CMAKE_PATH "$ENV{IDF_PATH}" ENV_IDF_PATH)

  if (EXISTS "${ENV_IDF_PATH}/components/freertos/include/freertos")
    set(FREERTOS_INCLUDES "${ENV_IDF_PATH}/components/freertos/include/freertos")
  else()
    if(CONFIG_FREERTOS_SMP)
      set(FREERTOS_INCLUDES "${ENV_IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/include/freertos")
    else()
      set(FREERTOS_INCLUDES "${ENV_IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos")
    endif()
  endif()

  idf_component_register(
    SRC_DIRS "FreeRTOS-Plus-POSIX/source"
    INCLUDE_DIRS
    "include"
    "include/private"
    "FreeRTOS-Plus-POSIX/include"
    "FreeRTOS-Plus-POSIX/include/portable/espressif/esp32_devkitc_esp_wrover_kit"
    "FreeRTOS-Plus-POSIX/include/portable"
    PRIV_INCLUDE_DIRS
    "include/private"
    "${FREERTOS_INCLUDES}"
  )

  set_source_files_properties("${freertos_plus_posix_dir}/source/FreeRTOS_POSIX_pthread_cond.c"
  PROPERTIES COMPILE_FLAGS
  "-Wno-maybe-uninitialized"
  )
endif()

