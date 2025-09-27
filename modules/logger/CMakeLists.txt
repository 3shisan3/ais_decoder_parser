project(logger)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/impl
)

# 使用logger
if (ENABLE_LOGGING)
    message(STATUS "Building with ENABLE_LOGGING")

    # 方案1：首先检查是否有现成的spdlog目标（由父项目编译）
    if (TARGET spdlog)
        message(STATUS "Using pre-compiled spdlog from parent project")
        
        target_link_libraries(${PROJECT_NAME} INTERFACE spdlog)
        target_compile_definitions(${PROJECT_NAME} INTERFACE LOGGING_SCHEME_SPDLOG)
    
    # 方案2：如果没有现成目标，尝试查找系统安装的spdlog
    else()
        find_package(spdlog QUIET)
        if (spdlog_FOUND)
            message(STATUS "Using system spdlog package")
            
            target_link_libraries(${PROJECT_NAME} INTERFACE spdlog::spdlog)
            target_compile_definitions(${PROJECT_NAME} INTERFACE LOGGING_SCHEME_SPDLOG)
        
        # 方案3：如果都没有，使用默认的fprintf方案
        else()
            message(STATUS "Using default fprintf logging scheme")
            # 不需要额外定义，默认使用logger_fprintf.h
        endif()
    endif()

    # 设置日志打印级别
    target_compile_definitions(${PROJECT_NAME} INTERFACE 
        $<$<CONFIG:Debug>:GLOBAL_LOG_LEVEL=0>
        $<$<NOT:$<CONFIG:Debug>>:GLOBAL_LOG_LEVEL=2>  # 非Debug模式
        $<$<BOOL:${ENABLE_LOGGING}>:ENABLE_LOGGING>
    )
endif()