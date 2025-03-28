idf_component_register(SRCS "test_io.c"
                       INCLUDE_DIRS "."
                       REQUIRES fatfs esp_adc logger_common
                       WHOLE_ARCHIVE)
