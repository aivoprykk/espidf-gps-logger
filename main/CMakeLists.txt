
set(pdir ${CMAKE_CURRENT_SOURCE_DIR}/../)

##second stage !important! after component register
if(CONFIG_LOGGER_BUILD_MODE_DEV)
MESSAGE("Dev mode selected")
set(ver "${CONFIG_LOGGER_VERSION_MAJOR}.${CONFIG_LOGGER_VERSION_MINOR}.${CONFIG_LOGGER_VERSION_PATCH}.${CONFIG_LOGGER_VERSION_BUILD}.dev")
set_source_files_properties(${srcs} PROPERTIES COMPILE_FLAGS "-DDEBUG")
write_file("${pdir}version.txt" "${ver}")
else()
if(CONFIG_LOGGER_BUILD_MODE_PROD)
MESSAGE("Prod mode selected")
set(ver "${CONFIG_LOGGER_VERSION_MAJOR}.${CONFIG_LOGGER_VERSION_MINOR}.${CONFIG_LOGGER_VERSION_PATCH}.${CONFIG_LOGGER_VERSION_BUILD}")
set_source_files_properties(${srcs} PROPERTIES COMPILE_FLAGS "-DNDEBUG")
write_file("${pdir}version.txt" "${ver}")
endif()
endif()

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
endif()

if(CONFIG_BMX_ENABLE)
set(srcs ${srcs} bmx.c)
endif()

idf_component_register(SRCS ${srcs} 
    INCLUDE_DIRS ${INCS} 
    REQUIRES ${REQ}
    PRIV_REQUIRES ${PRIVREQ}
)

UNSET(REQ)
UNSET(PRIVREQ)


MESSAGE("---- testing if flash_data exists in ${CMAKE_CURRENT_SOURCE_DIR}/../flash_data")
if(DEFINED CONFIG_IDF_TARGET_ESP32S3)
set(fdir ${pdir}/flash_data_s3)
else()
set(fdir ${pdir}/flash_data)
endif()
if(EXISTS ${fdir})
file(READ ${pdir}/${CONFIG_PARTITION_TABLE_CUSTOM_FILENAME} PARTS)
if(CONFIG_USE_FATFS)
string(REGEX MATCH "[^#]${CONFIG_FATFS_PARTITION_LABEL}" HAVE_FATFS_PART ${PARTS})
if(NOT ${HAVE_FATFS_PART} EQUAL -1)
    MESSAGE("---- label found, creating ${CONFIG_FATFS_PARTITION_LABEL} image")
    if(${CONFIG_FATFS_MODE_READ_ONLY})
        fatfs_create_rawflash_image(${CONFIG_FATFS_PARTITION_LABEL} ${fdir} FLASH_IN_PROJECT PRESERVE_TIME)
    else()
        fatfs_create_spiflash_image(${CONFIG_FATFS_PARTITION_LABEL} ${fdir} FLASH_IN_PROJECT PRESERVE_TIME)
    endif()
endif()
endif()
if(CONFIG_USE_SPIFFS)
string(REGEX MATCH "[^#]${CONFIG_SPIFFS_PARTITION_LABEL}" HAVE_SPIFFS_PART ${PARTS})
if(NOT ${HAVE_SPIFFS_PART} EQUAL -1)
    MESSAGE("---- label found, creating ${CONFIG_SPIFFS_PARTITION_LABEL} image")
    spiffs_create_partition_image(${CONFIG_SPIFFS_PARTITION_LABEL} ${fdir} FLASH_IN_PROJECT)
endif()
endif()
if(CONFIG_USE_LITTLEFS)
string(REGEX MATCH "[^#]${CONFIG_LITTLEFS_PARTITION_LABEL}" HAVE_LITTLEFS_PART ${PARTS})
if(NOT ${HAVE_LITTLEFS_PART} EQUAL -1)
    MESSAGE("---- label found, creating ${CONFIG_LITTLEFS_PARTITION_LABEL} image")
    littlefs_create_partition_image(${CONFIG_LITTLEFS_PARTITION_LABEL} ${fdir} FLASH_IN_PROJECT)
endif()
endif()
endif()
