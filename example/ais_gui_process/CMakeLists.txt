project(ais_decoder_parser_ui)

# 设置可执行程序输出路径
set(EXCUTABLE_OUTPUT_PATH ${AIS_SOURCES_ROOT}/bin)

include(${CMAKE_MODULE_PATH}/IncludeDirectories_LOG.cmake)

include_directories(
    ${AIS_SOURCES_ROOT}/example/ais_ipc
    
    ${CMAKE_CURRENT_LIST_DIR}/src
)

file(GLOB_RECURSE AIS_GUI_SOURCES
    ${AIS_SOURCES_ROOT}/example/ais_ipc/*.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main_gui.cpp
)

add_executable(ais_decoder_parser_ui ${AIS_GUI_SOURCES})

target_link_libraries(ais_decoder_parser_ui
    PRIVATE spdlog::spdlog
)

# 添加Windows Socket库链接
if(WIN32)
    target_link_libraries(ais_decoder_parser_ui
        PRIVATE ws2_32
    )
endif()