project(ais_config)

# 包含目录
include(${CMAKE_MODULE_PATH}/IncludeDirectories_CFG.cmake)

# 源文件
file(GLOB_RECURSE CONFIG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 构建库
add_library(ais_config STATIC ${CONFIG_SOURCES})

# 链接依赖库
target_link_libraries(ais_config PRIVATE
    yaml-cpp
)