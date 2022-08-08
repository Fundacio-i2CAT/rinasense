

file(TO_CMAKE_PATH "$ENV{IDF_PATH}" ENV_IDF_PATH)

# set(esp_idf_dir $ENV{IDF_PATH})
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
  "${ENV_IDF_PATH}/components/freertos/include/freertos"
)

# set_source_files_properties("${freertos_plus_posix_dir}/source/FreeRTOS_POSIX_pthread_cond.c"
# PROPERTIES COMPILE_FLAGS
# "-Wno-maybe-uninitialized"
# )
