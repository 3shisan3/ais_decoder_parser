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

# 构建后运行依赖打包命令
if(WIN32)
    if (MINGW)
        # 获取 UCRT64 工具链的 bin 目录
        get_filename_component(COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        get_filename_component(TOOLCHAIN_BIN_DIR "${COMPILER_DIR}/../bin" ABSOLUTE)
        
        # 根据工具链类型选择部署脚本
        if(CMAKE_CXX_COMPILER MATCHES ".*ucrt.*")
            # UCRT 工具链
            set(DEPLOY_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/script/tools/deploy_ucrt_dlls.sh")
            set(TOOLCHAIN_TYPE "UCRT64")
        else()
            # 标准 MinGW 工具链
            set(DEPLOY_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/script/tools/deploy_mingw_dlls.sh")
            set(TOOLCHAIN_TYPE "MinGW")
        endif()
        
        # 检查脚本是否存在
        if(EXISTS "${DEPLOY_SCRIPT}")
            # 构建后分析依赖并复制
            add_custom_command(TARGET ais_decoder_parser POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                COMMAND bash "${DEPLOY_SCRIPT}"
                    "${TOOLCHAIN_BIN_DIR}"
                    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                    "$<TARGET_FILE:ais_decoder_parser>"
                COMMENT "Copying required ${TOOLCHAIN_TYPE} runtime DLLs..."
                VERBATIM
            )
        else()
            message(WARNING "Deploy script not found: ${DEPLOY_SCRIPT}")
        endif()
    endif()
endif()