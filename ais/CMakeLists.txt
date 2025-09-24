
project(ais_parser)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 源文件
file(GLOB_RECURSE AIS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 构建库
add_library(ais_parser STATIC ${AIS_SOURCES})