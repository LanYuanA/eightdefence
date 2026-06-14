#!/bin/bash
# 编译脚本
set -e

ROOT_PWD=$( cd "$( dirname $0 )" && pwd )
BUILD_DIR=${ROOT_PWD}/build

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake .. -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_BUILD_TYPE=Release
make -j4

echo "编译完成！可执行文件位于: build/my_demo"
