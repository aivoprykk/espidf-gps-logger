
set(pdir ${CMAKE_CURRENT_SOURCE_DIR}/../)

##second stage !important! after component register

set(srcs
    "hid.c"
    "lcd.c"
    "main.c"
)

set(INCS
"${CMAKE_BINARY_DIR}/config"
"."
)

if(CONFIG_BMX_ENABLE)
set(srcs ${srcs} bmx.c)
endif()

if(CONFIG_DISPLAY_ENABLED)
if(CONFIG_DISPLAY_USE_LVGL)

SET(lvui_dir lvui)
FILE(GLOB UISRCS  ${lvui_dir}/*.c ${lvui_dir}/screens/*.c)
FILE(GLOB UIFONTS ${lvui_dir}/fonts/ui_font*.c)

if(CONFIG_DISPLAY_DRIVER_ST7789 OR CONFIG_DISPLAY_DRIVER_QEMU)
    FILE(GLOB UIIMAGES ${lvui_dir}/img16b/*.c)
else()
    FILE(GLOB UIIMAGES ${lvui_dir}/img8b/*.c)
endif()

SET(INCS ${INCS} ${lvui_dir}/include)

endif()
endif()



# Register ULP sources with the component
idf_component_register(SRCS ${srcs} ${UISRCS} ${UIFONTS} ${UIIMAGES}
    INCLUDE_DIRS ${INCS}
    PRIV_REQUIRES logger_common strutil logger_context logger_config
)

if(NOT ${IDF_TARGET} STREQUAL "linux")
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::driver)
endif()
if(CONFIG_BMX_ENABLE)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::esp-idf-bmx280)
endif(CONFIG_BMX_ENABLE)
if(CONFIG_LOGGER_BUTTON_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::logger_button)
endif(CONFIG_LOGGER_BUTTON_ENABLED)
if(CONFIG_GPS_LOG_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PUBLIC idf::gps_log)
endif(CONFIG_GPS_LOG_ENABLED)
if(CONFIG_UBLOX_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::logger_ubx)
endif(CONFIG_UBLOX_ENABLED)
if(CONFIG_DISPLAY_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::display)
endif()
if(CONFIG_LOGGER_ADC_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::logger_adc)
endif()
if(CONFIG_LOGGER_VFS_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::logger_vfs)
endif()
if(CONFIG_LOGGER_WIFI_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::logger_wifi)
endif()
if(CONFIG_LOGGER_HTTP_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::logger_http)
endif()


if(CONFIG_LOGGER_VFS_ENABLED AND CONFIG_FLASH_PARTITIONS)
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
endif()
