#!/bin/bash
# 运行脚本
cd "$(dirname "$0")"

MODEL=${1:-"model/deepseek-1.5b-w8a8-rk3588-16k.rkllm"}
MAX_NEW_TOKENS=${2:-2048}
MAX_CONTEXT_LEN=${3:-16384}

export LD_LIBRARY_PATH="$(pwd)/lib:${LD_LIBRARY_PATH}"
export RKLLM_LOG_LEVEL=1

echo "======================================"
echo " My DeepSeek Demo"
echo "======================================"
echo "模型: $MODEL"
echo "最大生成tokens: $MAX_NEW_TOKENS"
echo "最大上下文长度: $MAX_CONTEXT_LEN"
echo "======================================"

./build/my_demo "$MODEL" "$MAX_NEW_TOKENS" "$MAX_CONTEXT_LEN"
