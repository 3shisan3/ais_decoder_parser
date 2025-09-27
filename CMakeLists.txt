cmake_minimum_required(VERSION 3.10)

project(asi_decoder_parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

# 基础变量定义
set(AIS_SOURCES_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

set(MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)

# 构建前提
option(ENABLE_LOGGING "开启日志打印功能" ON)

add_subdirectory(${MODULES_DIR}/logger)
add_subdirectory(${MODULES_DIR}/ais)
add_subdirectory(${MODULES_DIR}/communicate)
add_subdirectory(${MODULES_DIR}/config)

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/ais_service_process)