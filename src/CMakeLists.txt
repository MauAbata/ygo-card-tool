# Define Main Sources
add_executable(${PROJECT_NAME} main.c)
target_sources(${PROJECT_NAME} PRIVATE hd.c ygo_nfc.c ygo_database.c cmd_write.c)

#target_compile_definitions(${PROJECT_NAME} PUBLIC -DLANG_JP)

target_include_directories(${PROJECT_NAME} PRIVATE ../include)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ygo")

# Link ygo-c
target_include_directories(${PROJECT_NAME} PRIVATE ../lib/ygo-c/include)
target_link_libraries(${PROJECT_NAME} PRIVATE ygo-c)

# Link cJSON
target_include_directories(${PROJECT_NAME} PRIVATE ../lib/cJSON)
target_link_libraries(${PROJECT_NAME} PRIVATE cjson)

# Link cURL
target_include_directories(${PROJECT_NAME} PRIVATE ../lib/curl/include)
target_link_libraries(${PROJECT_NAME} PRIVATE libcurl_static)

# Link libnfc
target_include_directories(${PROJECT_NAME} PRIVATE ../lib/libnfc/include ../lib/libnfc/utils)
target_link_libraries(${PROJECT_NAME} PRIVATE nfc)
target_sources(${PROJECT_NAME} PRIVATE ../lib/libnfc/utils/mifare.c)

# Link cargs
target_include_directories(${PROJECT_NAME} PRIVATE ../lib/cargs/include)
target_link_libraries(${PROJECT_NAME} PRIVATE cargs)
