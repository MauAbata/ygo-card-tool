cmake_minimum_required(VERSION 3.15)
project(ygo_card_tool C)

set(CMAKE_C_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON)
set(CJSON_BUILD_SHARED_LIBS OFF)

add_subdirectory(src)
add_subdirectory(lib/ygo-c)
add_subdirectory(lib/cJSON)

target_compile_options(cjson PRIVATE -Wno-float-conversion -fno-stack-protector)
