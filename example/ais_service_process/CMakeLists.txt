project(ais_decoder_parser)

# 设置可执行程序输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AIS_SOURCES_ROOT}/bin/ais_decoder_parser)

include(${CMAKE_MODULE_PATH}/IncludeDirectories_COMM.cmake)
include_directories(
    ${AIS_SOURCES_ROOT}/example/ais_ipc

    ${CMAKE_CURRENT_LIST_DIR}/src
)

file(GLOB_RECURSE AIS_SERVICE_SOURCES
    ${AIS_SOURCES_ROOT}/example/ais_ipc/*.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main_service.cpp
)

add_executable(ais_decoder_parser ${AIS_SERVICE_SOURCES})

target_link_libraries(ais_decoder_parser
    PRIVATE ais_parser_communicate
    PRIVATE ais_config
)
