cmake_minimum_required(VERSION 3.10)

project(asi_decoder_parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 基础变量定义
set(AIS_SOURCES_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

add_subdirectory(logger)

add_subdirectory(ais)