project(ais_parser_communicate)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/udp-tcp-communicate/CMakeLists.txt")
    message(STATUS "  Using local udp-tcp-communicate from ${CMAKE_CURRENT_LIST_DIR}/udp-tcp-communicate")
            
    # 设置动态库选项传递给子项目
    set(UDPTCP_BUILD_SHARED ${UDPTCP_USE_SHARED} PARENT_SCOPE)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/udp-tcp-communicate" udp-tcp-communicate)
else()
    message(STATUS "  Using FetchContent for udp-tcp-communicate")
    include(FetchContent)
    FetchContent_Declare(
        udp-tcp-communicate
        GIT_REPOSITORY https://github.com/3shisan3/udp-tcp-communicate.git
        GIT_TAG main
        GIT_SUBMODULES_RECURSE ON  # 递归拉取所有子模块
    )
        
    # 设置动态库选项
    set(BUILD_SHARED_LIBS ${UDPTCP_USE_SHARED})
    FetchContent_MakeAvailable(udp-tcp-communicate)
endif()

# 包含目录
include(${CMAKE_MODULE_PATH}/IncludeDirectories_COMM.cmake)

# 源文件
file(GLOB_RECURSE AIS_COMMUNICATE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

add_library(ais_parser_communicate STATIC ${AIS_COMMUNICATE_SOURCES})

target_link_libraries(ais_parser_communicate udp-tcp-communicate)

