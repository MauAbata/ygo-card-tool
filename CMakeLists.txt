cmake_minimum_required(VERSION 3.15)
project(ygo_card_tool C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(src)
add_subdirectory(lib/ygo-c)

target_include_directories(ygo_card_tool PRIVATE lib/ygo-c/include)
target_link_libraries(ygo_card_tool PRIVATE ygo-c)