project(ais_decoder_parser_ui)

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
find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)

# 设置Qt的依赖库
set(QT_DEPEND_LIBS
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

# 设置可执行程序输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AIS_SOURCES_ROOT}/bin)

# 引入 Qt 会干扰 CMAKE_MODULE_PATH
include(${AIS_SOURCES_ROOT}/cmake/IncludeDirectories_LOG.cmake)

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
    PRIVATE logger
    PRIVATE ${QT_DEPEND_LIBS}
)

# 添加Windows Socket库链接
if(WIN32)
    target_link_libraries(ais_decoder_parser_ui
        PRIVATE ws2_32
    )
endif()