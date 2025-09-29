project(ais_test_task)

# 设置默认的 Qt 目录
set(QT_PREFIX_PATH "D:\\Dev\\Qt\\6.8.3\\mingw_64" CACHE PATH "Default Qt install path")

# 检查路径是否存在
if(NOT EXISTS ${QT_PREFIX_PATH})
    message(FATAL_ERROR "QT_PREFIX_PATH: ${QT_PREFIX_PATH} does not exist.")
endif()

# 将 Qt 路径添加到搜索目录中
list(APPEND CMAKE_PREFIX_PATH ${QT_PREFIX_PATH})

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
        get_filename_component(UCRT_BIN_DIR "${COMPILER_DIR}/../bin" ABSOLUTE)
        
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
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/ui/qml"
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