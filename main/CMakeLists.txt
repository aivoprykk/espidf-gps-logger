
set(srcs
"main.c"
"dstat_screens.c"
)

set(INCS
"${CMAKE_BINARY_DIR}/config"
"."
)

set(REQ gps_log)
set(PRIVREQ driver esp_wifi
 display logger_common logger_context logger_button logger_vfs logger_str
 logger_config logger_ubx logger_adc logger_wifi logger_http)

if(CONFIG_DISPLAY_USE_LVGL)
set(srcs ${srcs} lcd.c)
else()
set(srcs ${srcs} epaper.c)
endif()

if(CONFIG_BMX_ENABLE)
set(srcs ${srcs} bmx.c)
endif()

if(CONFIG_OTA_USE_AUTO_UPDATE)
set(ota_crt ${CMAKE_CURRENT_SOURCE_DIR}/../certs/majasa_ca.pem)
endif()


idf_component_register(SRCS ${srcs} 
    INCLUDE_DIRS ${INCS} 
    #EMBED_TXTFILES ${ota_crt} 
    REQUIRES ${REQ}
    PRIV_REQUIRES ${PRIVREQ}
)

UNSET(REQ)
UNSET(PRIVREQ)

##second stage !important! after component register
if(CONFIG_VERBOSE_BUILD)
MESSAGE("VERBOSE_BUILD selected")
set_source_files_properties(${srcs} PROPERTIES COMPILE_FLAGS "-DDEBUG")
#target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLOG_LOCAL_LEVEL=ESP_LOG_VERBOSE")
else()
set_source_files_properties(${srcs} PROPERTIES COMPILE_FLAGS "-DNDEBUG")
#target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLOG_LOCAL_LEVEL=ESP_LOG_WARNING")
endif()

if(EXISTS ${PROJECT_SOURCE_DIR}/flash_data)
file(READ ${PROJECT_SOURCE_DIR}/${CONFIG_PARTITION_TABLE_FILENAME} PARTS)
string(FIND "${PARTS}" ${CONFIG_FATFS_PARTITION_LABEL} HAVE_FATFS_PART)
if(CONFIG_USE_FATFS AND NOT ${HAVE_FATFS_PART} EQUAL -1)
    fatfs_create_rawflash_image(${CONFIG_FATFS_PARTITION_LABEL} ${PROJECT_SOURCE_DIR}/flash_data FLASH_IN_PROJECT PRESERVE_TIME)
endif()
string(FIND "${PARTS}" ${CONFIG_SPIFFS_PARTITION_LABEL} HAVE_SPIFFS_PART)
if(CONFIG_USE_SPIFFS AND NOT ${HAVE_SPIFFS_PART} EQUAL -1)
    spiffs_create_partition_image(${CONFIG_SPIFFS_PARTITION_LABEL} ${PROJECT_SOURCE_DIR}/flash_data FLASH_IN_PROJECT)
endif()
string(FIND "${PARTS}" ${CONFIG_SPIFFS_PARTITION_LABEL} HAVE_LITTLEFS_PART)
if(CONFIG_USE_LITTLEFS AND NOT ${HAVE_LITTLEFS_PART} EQUAL -1)
    littlefs_create_partition_image(${CONFIG_LITTLEFS_PARTITION_LABEL} ${PROJECT_SOURCE_DIR}/flash_data FLASH_IN_PROJECT)
endif()
endif()
# if(CONFIG_WEB_DEPLOY_SF)
# if(${CONFIG_WEB_DEPLOY_PARTITION} STREQUAL "sdcard")
#     set(WEB_SRC_DIR "${PROJECT_SOURCE_DIR}/web-frontend")
#     if(EXISTS ${WEB_SRC_DIR}/dist)
#         fatfs_create_rawflash_image(${CONFIG_WEB_PARTITION_LABEL} ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT PRESERVE_TIME)
#         #fatfs_create_spiflash_image(WEB_PART_LABEL ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT PRESERVE_TIME)
#     else()
#         message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
#     endif()
# endif()
# endif()
