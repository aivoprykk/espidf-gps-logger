
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
else()
set_source_files_properties(${srcs} PROPERTIES COMPILE_FLAGS "-DNDEBUG")
endif()

MESSAGE("---- testing if flash_data exists in ${CMAKE_CURRENT_SOURCE_DIR}/../flash_data")
set(pdir ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(fdir ${pdir}/flash_data)
if(EXISTS ${fdir})
file(READ ${pdir}/${CONFIG_PARTITION_TABLE_FILENAME} PARTS)
if(CONFIG_USE_FATFS)
string(FIND "${PARTS}" ${CONFIG_FATFS_PARTITION_LABEL} HAVE_FATFS_PART)
MESSAGE("---- test if flash fatfs partition ${HAVE_FATFS_PART}")
if(NOT ${HAVE_FATFS_PART} EQUAL -1)
    fatfs_create_rawflash_image(${CONFIG_FATFS_PARTITION_LABEL} ${fdir} FLASH_IN_PROJECT PRESERVE_TIME)
endif()
endif()
if(CONFIG_USE_SPIFFS)
string(FIND "${PARTS}" ${CONFIG_SPIFFS_PARTITION_LABEL} HAVE_SPIFFS_PART)
MESSAGE("---- test if flash spiffs partition ${HAVE_SPIFFS_PART}")
if(NOT ${HAVE_SPIFFS_PART} EQUAL -1)
    spiffs_create_partition_image(${CONFIG_SPIFFS_PARTITION_LABEL} ${fdir} FLASH_IN_PROJECT)
endif()
endif()
if(CONFIG_USE_LITTLEFS)
string(FIND "${PARTS}" ${CONFIG_LITTLEFS_PARTITION_LABEL} HAVE_LITTLEFS_PART)
MESSAGE("---- test if flash littlefs partition ${HAVE_LITTLEFS_PART}")
if(NOT ${HAVE_LITTLEFS_PART} EQUAL -1)
    littlefs_create_partition_image(${CONFIG_LITTLEFS_PARTITION_LABEL} ${fdir} FLASH_IN_PROJECT)
endif()
endif()
endif()
