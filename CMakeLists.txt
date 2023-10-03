cmake_minimum_required(VERSION 3.15)
project(ygo_card_tool C)

set(CMAKE_C_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

# cJSON Options
set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON CACHE BOOL "")
set(CJSON_BUILD_SHARED_LIBS OFF CACHE BOOL "")

# cURL Options
set(ENABLE_UNICODE ON CACHE BOOL "")
#set(CURL_STATIC_CRT ON CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(BUILD_STATIC_LIBS ON CACHE BOOL "")
set(BUILD_CURL_EXE OFF CACHE BOOL "")
set(CURL_ENABLE_SSL ON CACHE BOOL "")

if(WIN32)
    set(CURL_USE_SCHANNEL ON CACHE BOOL "")
endif()

set(HTTP_ONLY ON CACHE BOOL "")
set(ENABLE_IPV6 OFF CACHE BOOL "")
#set(CURL_LTO ON CACHE BOOL "")

add_definitions("-DCURL_STATICLIB -DHTTP_ONLY")

# This is for libnfc, which errors with:
# Policy CMP0115 is not set: Source file extensions must be explicit.
set(CMAKE_POLICY_DEFAULT_CMP0115 OLD)
set(CMAKE_POLICY_DEFAULT_CMP0042 OLD)
#cmake_policy(SET CMP0115 OLD)

add_subdirectory(lib/ygo-c)
add_subdirectory(lib/cJSON)
add_subdirectory(lib/curl)
add_subdirectory(lib/libnfc)
add_subdirectory(src)

target_compile_options(cjson PRIVATE -Wno-float-conversion -fno-stack-protector)
