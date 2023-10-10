cmake_minimum_required(VERSION 3.15)
project(ygo_card_tool C)

set(CMAKE_C_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

# cJSON Options
set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON CACHE BOOL "")
set(CJSON_BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(ENABLE_CUSTOM_COMPILER_FLAGS OFF CACHE BOOL "")

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

if(APPLE)
    # Mac has a constantly running daemon that locks the USB device. The fix is very ugly, so
    # hopefully we can just comply with the daemon to access it instead. Ugh. Troublesome.
    set(LIBNFC_DRIVER_ACR122_PCSC ON CACHE BOOL "")
    set(LIBNFC_DRIVER_ACR122_USB OFF CACHE BOOL "")
endif()

add_definitions("-DCURL_STATICLIB -DHTTP_ONLY")

# This is for libnfc, which errors with:
# Policy CMP0115 is not set: Source file extensions must be explicit.
set(CMAKE_POLICY_DEFAULT_CMP0115 OLD)
set(CMAKE_POLICY_DEFAULT_CMP0042 OLD)
#cmake_policy(SET CMP0115 OLD)

# This if for libnfc
set(BUILD_EXAMPLES OFF CACHE BOOL "")
set(BUILD_UTILS OFF CACHE BOOL "")

message("=== CMAKE REBUILT ===")

add_subdirectory(lib/ygo-c)
add_subdirectory(lib/cJSON)
add_subdirectory(lib/curl)

if(WIN32)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

    # We're going to manually bundle in this nonsense...
    FIND_PATH(LIBUSB_INCLUDE_DIRS lusb0_usb.h "${CMAKE_CURRENT_SOURCE_DIR}/lib/libusb-win32/include" NO_SYSTEM_ENVIRONMENT_PATH)
    FIND_LIBRARY(LIBUSB_LIBRARIES NAMES libusb PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib/libusb-win32/lib/msvc" REQUIRED)
#    SET(LIBUSB_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/libusb-win32/bin/x86/")
#    STRING(REGEX REPLACE "\\\\" "/" LIBUSB_LIBRARY_DIR ${LIBUSB_LIBRARY_DIR})
    SET(LIBUSB_FOUND ON)
endif()

add_subdirectory(lib/libnfc)
add_subdirectory(lib/cargs)
add_subdirectory(src)

#target_compile_options(cjson PRIVATE -Wno-float-conversion -fno-stack-protector)

if(WIN32)
#    target_compile_options(nfc PRIVATE -Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Wno-format)
    # This is a strange name for it, but setting _WINDLL=1 for whatever reason disables the declspec for libnfc's header...
    target_compile_definitions(nfc PRIVATE -DBUILD_STATIC_LIBS=1)
    target_compile_definitions(ygo_card_tool PRIVATE -DBUILD_STATIC_LIBS=1)
    # Now we just gotta build libusb-64 bit
endif()
