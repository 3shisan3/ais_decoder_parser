project(ais_test_task)

# 设置默认Qt安装路径
set(QT_PREFIX_PATH "" CACHE PATH "Default Qt install path")

# 如果未设置 QT_PREFIX_PATH，尝试自动查找
if(NOT QT_PREFIX_PATH)
    # 方法1：从环境变量查找
    if(DEFINED ENV{QTDIR})
        set(QT_PREFIX_PATH "$ENV{QTDIR}")
        message(STATUS "Using QTDIR environment variable: ${QT_PREFIX_PATH}")
    # 方法2：从编译器路径推断（适用于 MinGW）
    elseif(CMAKE_CXX_COMPILER MATCHES ".*msys.*")
        get_filename_component(COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        get_filename_component(POSSIBLE_QT_DIR "${COMPILER_DIR}/../" ABSOLUTE)

        set(QT_PREFIX_PATH "${POSSIBLE_QT_DIR}")
        message(STATUS "Found Qt in MinGW directory: ${QT_PREFIX_PATH}")
    # 方法3：在常见安装位置查找
    else()
        set(COMMON_QT_PATHS
            "C:/Qt"
            "$ENV{USERPROFILE}/Qt"
            "C:/Program Files/Qt"
        )
        foreach(qt_path IN LISTS COMMON_QT_PATHS)
            if(EXISTS "${qt_path}")
                set(QT_PREFIX_PATH "${qt_path}")
                message(STATUS "Found Qt in common location: ${QT_PREFIX_PATH}")
                break()
            endif()
        endforeach()
    endif()
    
    # 如果仍然未找到，设置默认值但不报错，让后续的 find_package 处理
    if(NOT QT_PREFIX_PATH)
        message(STATUS "QT_PREFIX_PATH not set, relying on system PATH for Qt discovery")
    endif()
endif()

# 检查路径是否存在（仅在路径不为空时检查）
if(QT_PREFIX_PATH AND NOT EXISTS ${QT_PREFIX_PATH})
    message(WARNING "QT_PREFIX_PATH: '${QT_PREFIX_PATH}' does not exist. Relying on system PATH.")
    unset(QT_PREFIX_PATH CACHE)
endif()

# 将 Qt 路径添加到搜索目录中
if(QT_PREFIX_PATH)
    list(APPEND CMAKE_PREFIX_PATH ${QT_PREFIX_PATH})
endif()
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# 设置MOC，如果要使用信号槽，则必须开启该功能
set(CMAKE_AUTOMOC ON)

# 查找Qt依赖
find_package(Qt6 COMPONENTS Core Widgets Gui Network Positioning REQUIRED)

# 设置Qt的依赖库
set(QT_DEPEND_LIBS
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    Qt6::Positioning
)

# 设置可执行程序输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AIS_SOURCES_ROOT}/bin/ais_test_task)

include(${AIS_SOURCES_ROOT}/cmake/IncludeDirectories_CFG.cmake)
include(${AIS_SOURCES_ROOT}/cmake/IncludeDirectories_AIS.cmake)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/src
)

file(GLOB_RECURSE AIS_TEST_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

add_executable(ais_test_task WIN32 ${AIS_TEST_SOURCES})

target_link_libraries(ais_test_task
    PRIVATE ${QT_DEPEND_LIBS}
    PRIVATE udp-tcp-communicate
    PRIVATE ais_parser
)

# 添加Windows Socket库链接
if(WIN32)
    target_link_libraries(ais_test_task
        PRIVATE ws2_32
    )
endif()

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
            add_custom_command(TARGET ais_test_task POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                COMMAND bash "${DEPLOY_SCRIPT}"
                    "${TOOLCHAIN_BIN_DIR}"
                    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                    "$<TARGET_FILE:ais_test_task>"
                COMMENT "Copying required ${TOOLCHAIN_TYPE} runtime DLLs..."
                VERBATIM
            )
        else()
            message(WARNING "Deploy script not found: ${DEPLOY_SCRIPT}")
        endif()
    endif()

    # Qt 部署
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt.exe
        HINTS "${QT_PREFIX_PATH}/bin"
        DOC "Path to windeployqt executable"
    )
    
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ais_test_task POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-compiler-runtime
                --no-system-d3d-compiler
                --no-opengl-sw
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ais_test_task.exe"
            COMMENT "Deploying Qt runtime dependencies..."
            VERBATIM
        )
    else()
        message(WARNING "windeployqt.exe not found, Qt deployment skipped")
    endif()
endif()