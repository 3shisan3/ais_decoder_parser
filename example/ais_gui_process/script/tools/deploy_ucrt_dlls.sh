#!/bin/bash

# 参数检查
if [ $# -ne 3 ]; then
    echo "Usage: $0 <ucrt_bin_dir> <target_dir> <exe_path>"
    exit 1
fi

UCRT_BIN_DIR=$1
TARGET_DIR=$2
EXE_PATH=$3

# 创建目标目录
mkdir -p "${TARGET_DIR}"

# 使用 ldd 分析依赖（扩大搜索范围）
echo "Analyzing dependencies for ${EXE_PATH}..."
DEPENDENCIES=$(ldd "${EXE_PATH}" | awk '$3 ~ /mingw64|ucrt64|clang64|msys64/ {print $3}' | sort -u)

# 如果没有找到依赖，尝试使用objdump作为备用方案
if [ -z "${DEPENDENCIES}" ]; then
    echo "Using objdump as fallback for dependency analysis..."
    DEPENDENCIES=$(objdump -p "${EXE_PATH}" | grep "DLL Name:" | awk '{print $3}' | sort -u)
fi

# 打印识别到的依赖库（调试用）
echo "Identified dependency libraries:"
printf '%s\n' "${DEPENDENCIES[@]}"
echo "============================"

# 复制依赖的 DLL
COPIED_COUNT=0
for DLL in ${DEPENDENCIES}; do
    DLL_NAME=$(basename "${DLL}")
    
    # 首先尝试从原始路径复制
    if [ -f "${DLL}" ]; then
        cp -v "${DLL}" "${TARGET_DIR}"
        COPIED_COUNT=$((COPIED_COUNT + 1))
    else
        # 如果在原路径找不到，尝试在ucrt64目录下查找
        UCRT_DLL="${UCRT_BIN_DIR}/${DLL_NAME}"
        if [ -f "${UCRT_DLL}" ]; then
            cp -v "${UCRT_DLL}" "${TARGET_DIR}"
            COPIED_COUNT=$((COPIED_COUNT + 1))
        else
            echo "Warning: Missing DLL - ${DLL_NAME}"
        fi
    fi
done

# 确保复制核心运行时库
CORE_DLLS=(
    "libstdc++-6.dll"
    "libgcc_s_seh-1.dll"
    "libwinpthread-1.dll"
    "libgcc_s_dw2-1.dll"
)

for CORE_DLL in "${CORE_DLLS[@]}"; do
    if [ -f "${UCRT_BIN_DIR}/${CORE_DLL}" ] && [ ! -f "${TARGET_DIR}/${CORE_DLL}" ]; then
        cp -v "${UCRT_BIN_DIR}/${CORE_DLL}" "${TARGET_DIR}"
        COPIED_COUNT=$((COPIED_COUNT + 1))
        echo "Added core DLL: ${CORE_DLL}"
    fi
done

# 特殊处理：查找并复制 spdlog DLL
echo "Searching for spdlog DLLs..."
# 在编译器目录中查找 spdlog DLL
find "${UCRT_BIN_DIR}" -name "libspdlog*.dll" -type f | while read SPDLOG_DLL; do
    SPDLOG_NAME=$(basename "${SPDLOG_DLL}")
    if [ ! -f "${TARGET_DIR}/${SPDLOG_NAME}" ]; then
        cp -v "${SPDLOG_DLL}" "${TARGET_DIR}"
        COPIED_COUNT=$((COPIED_COUNT + 1))
        echo "Added spdlog DLL: ${SPDLOG_NAME}"
    fi
done

# 特殊处理：API Set DLLs 映射
# 这些是 Windows 虚拟 DLL，实际功能在 ucrtbase.dll 中
API_SET_DLLS=(
    "api-ms-win-crt-convert-l1-1-0.dll"
    "api-ms-win-crt-environment-l1-1-0.dll"
    "api-ms-win-crt-heap-l1-1-0.dll"
    "api-ms-win-crt-locale-l1-1-0.dll"
    "api-ms-win-crt-math-l1-1-0.dll"
    "api-ms-win-crt-private-l1-1-0.dll"
    "api-ms-win-crt-runtime-l1-1-0.dll"
    "api-ms-win-crt-stdio-l1-1-0.dll"
    "api-ms-win-crt-string-l1-1-0.dll"
    "api-ms-win-crt-time-l1-1-0.dll"
    "api-ms-win-crt-utility-l1-1-0.dll"
)

# 检查是否需要 API Set DLLs
NEEDS_API_SET=false
for API_DLL in "${API_SET_DLLS[@]}"; do
    if ldd "${EXE_PATH}" | grep -q "${API_DLL}"; then
        NEEDS_API_SET=true
        break
    fi
done

# 如果程序需要 API Set DLLs，尝试从 Windows Kit 或创建符号链接
if [ "${NEEDS_API_SET}" = true ]; then
    echo "Program requires API Set DLLs, checking Windows Kit..."
    
    # 尝试在常见 Windows Kit 位置查找
    WINDOWS_KIT_PATHS=(
        "/c/Program Files (x86)/Windows Kits/10/Redist/ucrt/DLLs/x64"
        "/c/Program Files/Windows Kits/10/Redist/ucrt/DLLs/x64"
        "$ENV{USERPROFILE}/AppData/Local/Microsoft/Windows Kits/10/Redist/ucrt/DLLs/x64"
    )
    
    for KIT_PATH in "${WINDOWS_KIT_PATHS[@]}"; do
        if [ -d "${KIT_PATH}" ]; then
            echo "Found Windows Kit at: ${KIT_PATH}"
            for API_DLL in "${API_SET_DLLS[@]}"; do
                if [ -f "${KIT_PATH}/${API_DLL}" ] && [ ! -f "${TARGET_DIR}/${API_DLL}" ]; then
                    cp -v "${KIT_PATH}/${API_DLL}" "${TARGET_DIR}"
                    COPIED_COUNT=$((COPIED_COUNT + 1))
                fi
            done
            break
        fi
    done
    
    # 如果没找到 Windows Kit，创建警告信息
    if [ ! -f "${TARGET_DIR}/api-ms-win-crt-runtime-l1-1-0.dll" ]; then
        echo "Warning: API Set DLLs not found. These are usually provided by Windows."
        echo "Note: API Set DLLs are virtual DLLs that redirect to ucrtbase.dll in Windows 10+"
        echo "If running on Windows 7/8, you may need to install Visual C++ Redistributable"
    fi
fi

# 额外检查：查找项目依赖的其他 DLL
echo "Searching for additional project dependencies..."
PROJECT_DEPS=(
    "libspdlog.dll"
    "libspdlogd.dll"
    "logger.dll"
    "ais_parser.dll"
)

for DEP in "${PROJECT_DEPS[@]}"; do
    # 在编译器目录查找
    if [ -f "${UCRT_BIN_DIR}/${DEP}" ] && [ ! -f "${TARGET_DIR}/${DEP}" ]; then
        cp -v "${UCRT_BIN_DIR}/${DEP}" "${TARGET_DIR}"
        COPIED_COUNT=$((COPIED_COUNT + 1))
    fi
    
    # 在项目构建目录中查找
    BUILD_DIR=$(dirname "${EXE_PATH}")
    if [ -f "${BUILD_DIR}/${DEP}" ] && [ ! -f "${TARGET_DIR}/${DEP}" ]; then
        cp -v "${BUILD_DIR}/${DEP}" "${TARGET_DIR}"
        COPIED_COUNT=$((COPIED_COUNT + 1))
    fi
done

echo "UCRT64 DLL deployment completed. Copied ${COPIED_COUNT} DLLs."

# 最终验证
echo "Final dependency check:"
ldd "${EXE_PATH}" | grep "not found" && echo "Warning: Some dependencies are still missing" || echo "All dependencies resolved"