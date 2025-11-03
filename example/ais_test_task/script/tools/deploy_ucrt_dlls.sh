#!/bin/bash

# 参数检查
if [ $# -ne 3 ]; then
    echo "Usage: $0 <ucrt_bin_dir> <target_dir> <exe_path>"
    exit 1
fi

# 打印输入参数(调试使用)
# echo "============ input arguments ============"
# echo "UCRT Dir:   $1"
# echo "Target Dir: $2"
# echo "The EXE File: $3"
# echo "=================================="

UCRT_BIN_DIR=$1
TARGET_DIR=$2
EXE_PATH=$3

# 创建目标目录
mkdir -p "${TARGET_DIR}"

# 使用 ldd 分析依赖
echo "Analyzing dependencies for ${EXE_PATH}..."
# 改进过滤逻辑：匹配包含 ucrt64 的路径，或者来自 UCRT_BIN_DIR 的依赖
DEPENDENCIES=$(ldd "${EXE_PATH}" | awk -v ucrt_dir="${UCRT_BIN_DIR}" '
    $3 ~ /ucrt64|\/ucrt64\// || index($3, ucrt_dir) > 0 {print $3}
' | sort -u)

# 如果没有找到依赖，尝试更宽松的匹配
if [ -z "${DEPENDENCIES}" ]; then
    echo "Using broader dependency analysis..."
    DEPENDENCIES=$(ldd "${EXE_PATH}" | awk '$3 ~ /\.dll$/ {print $3}' | sort -u)
fi

# 打印识别到的依赖库（调试时取消注释）
# echo "Identified dependency libraries:"
# printf '%s\n' "${DEPENDENCIES[@]}"
# echo "============================"

# 复制依赖的 DLL
COPIED_COUNT=0
for DLL in ${DEPENDENCIES}; do
    DLL_NAME=$(basename "${DLL}")
    # echo "Copying ${DLL_NAME}"

    if [ -f "${DLL}" ]; then
        cp -v "${DLL}" "${TARGET_DIR}"
        COPIED_COUNT=$((COPIED_COUNT + 1))
    else
        # 如果在原路径找不到，尝试在UCRT目录下查找
        UCRT_DLL="${UCRT_BIN_DIR}/${DLL_NAME}"
        if [ -f "${UCRT_DLL}" ]; then
            cp -v "${UCRT_DLL}" "${TARGET_DIR}"
            COPIED_COUNT=$((COPIED_COUNT + 1))
        else
            echo "Warning: Missing DLL - ${DLL} And ${UCRT_DLL}"
        fi
    fi
done

# 额外检查并复制常见的 UCRT 核心库（确保所有必要库都被复制）
UCRT_CORE_DLLS=(
    "libgcc_s_seh-1.dll"
    "libwinpthread-1.dll"
    "libstdc++-6.dll"
    "libgcc_s_dw2-1.dll"
    "libb2-1.dll"
    "zlib1.dll"
    "libdouble-conversion.dll"
    "libzstd.dll"
    "libpcre2-16-0.dll"
    "libicuuc77.dll"
    "libicuin77.dll"
    "libmd4c.dll"
    "libbrotlidec.dll"
    "libfreetype-6.dll"
    "libicudt77.dll"
    "libharfbuzz-0.dll"
    "libpng16-16.dll"
    "libbrotlicommon.dll"
    "libbz2-1.dll"
    "libgraphite2.dll"
    "libglib-2.0-0.dll"
    "libintl-8.dll"
    "libiconv-2.dll"
    "libpcre2-8-0.dll"
)

echo "Checking for additional core DLLs..."
for CORE_DLL in "${UCRT_CORE_DLLS[@]}"; do
    if [ -f "${UCRT_BIN_DIR}/${CORE_DLL}" ] && [ ! -f "${TARGET_DIR}/${CORE_DLL}" ]; then
        cp -v "${UCRT_BIN_DIR}/${CORE_DLL}" "${TARGET_DIR}"
        COPIED_COUNT=$((COPIED_COUNT + 1))
        echo "Additional DLL copied: ${CORE_DLL}"
    fi
done

echo "UCRT64 DLL deployment completed. Copied ${COPIED_COUNT} DLLs."