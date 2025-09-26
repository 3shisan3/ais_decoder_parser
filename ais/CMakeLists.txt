
project(ais_parser)

# 包含目录
include(${CMAKE_MODULE_PATH}/IncludeDirectories_CFG.cmake)
include(${CMAKE_MODULE_PATH}/IncludeDirectories_LOG.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
file(GLOB_RECURSE AIS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 构建库
add_library(ais_parser STATIC ${AIS_SOURCES})