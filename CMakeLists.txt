# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# (Not part of the boilerplate)
# This example uses an extra component for common functions such as Wi-Fi and Ethernet connection.
#list(APPEND EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Disable annoying warnings (for now)
idf_build_set_property(COMPILE_OPTIONS "-Wno-missing-field-initializers" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-unused-variable" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-unused-function" APPEND)

idf_build_set_property(COMPILE_OPTIONS "-Wno-pointer-arith" APPEND) 

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/version.txt")
write_file("${CMAKE_SOURCE_DIR}/version.txt" "1.0.4.2")
endif()

file(READ "${CMAKE_SOURCE_DIR}/version.txt" VER)

string(REGEX MATCH "([0-9]*)\\.([0-9]*)\\.([0-9]*)\\.([0-9]*)(\\.[a-z]+)?" _ ${VER})
set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
set(PROJECT_VERSION_TWEAK ${CMAKE_MATCH_4})
set(PROJECT_MODE ${CMAKE_MATCH_5})
if("${PROJECT_MODE}" STRGREATER_EQUAL ".dev")
MESSAGE(STATUS "Dev mode selected")
idf_build_set_property(COMPILE_OPTIONS "-DDEBUG" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DBUILD_MODE_DEV" APPEND)
else()
MESSAGE(STATUS "Prod mode selected")
idf_build_set_property(COMPILE_OPTIONS "-DNDEBUG" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DBUILD_MODE_PROD" APPEND)
endif()

idf_build_set_property(COMPILE_OPTIONS "-DVERSION_MAJOR=${PROJECT_VERSION_MAJOR}" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DVERSION_MINOR=${PROJECT_VERSION_MINOR}" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DVERSION_PATCH=${PROJECT_VERSION_PATCH}" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DVERSION_TWEAK=${PROJECT_VERSION_TWEAK}" APPEND)

set(PROJECT_NAME "espidf-gps-logger")
set(PROJECT_VER "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}${PROJECT_MODE}")
set(PROJECT_VER_NUM "${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}${PROJECT_VERSION_TWEAK}${PROJECT_MODE}")
project(${PROJECT_NAME}-${PROJECT_VER_NUM})



# Enable colors for compile output
idf_build_set_property(COMPILE_OPTIONS "-fdiagnostics-color=always" APPEND)

# Display Compile Time Information
message(STATUS "--------------Compile Info------------")
message(STATUS "IDF_PATH = ${IDF_PATH}")
message(STATUS "IDF_TARGET = ${IDF_TARGET}")
message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")
message(STATUS "PROJECT_VER = ${PROJECT_VER}")
message(STATUS "PROJECT_DIR = ${PROJECT_DIR}")
message(STATUS "BUILD_DIR = ${BUILD_DIR}")
message(STATUS "SDKCONFIG = ${SDKCONFIG}")
message(STATUS "SDKCONFIG_DEFAULTS = ${SDKCONFIG_DEFAULTS}")
message(STATUS "CONFIG_LV_CONF_SKIP = ${CONFIG_LV_CONF_SKIP}")
message(STATUS "---------------------------------------")
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message(STATUS "---------------------------------------")