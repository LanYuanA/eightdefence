# My DeepSeek Demo

基于 RKLLM 的 DeepSeek-R1-Distill-Qwen-1.5B 模型推理 Demo。

## 目录结构

```
my_demo/
├── src/           # 源代码目录
│   └── main.cpp   # 主程序
├── lib/           # 库文件目录
│   ├── rkllm.h    # 头文件
│   └── librkllmrt.so  # 运行时库 (v1.1.4)
├── model/         # 模型文件目录
│   └── deepseek-1.5b-w8a8-rk3588-16k.rkllm -> 符号链接
├── build/         # 编译输出目录
├── build.sh       # 编译脚本
├── run.sh         # 运行脚本
├── CMakeLists.txt # CMake 配置文件
└── README.md      # 本文件
```

## 编译

```bash
./build.sh
```

## 运行

```bash
# 使用默认模型和参数
./run.sh

# 自定义模型路径和参数
./run.sh /path/to/model.rkllm 2048 16384
```

## 参数说明

- `model_path`: 模型文件路径
- `max_new_tokens`: 最大生成 token 数量
- `max_context_len`: 最大上下文长度

## 注意事项

- 使用 rkllm-runtime 1.1.4 版本（解决了 1.2.3 版本的 bug）
- 当前为单轮对话模式
- 输入 `exit` 退出程序

## 自定义开发

修改 `src/main.cpp` 文件后重新编译即可。

## 关键代码说明

### 对话模板
```cpp
#define PROMPT_TEXT_PREFIX "<｜begin▁of▁sentence｜><｜User｜>"
#define PROMPT_TEXT_POSTFIX "<｜Assistant｜>"
```

### 拼接 Prompt
```cpp
string text = PROMPT_TEXT_PREFIX + input_str + PROMPT_TEXT_POSTFIX;
rkllm_input.prompt_input = (char *)text.c_str();
```

### 运行推理
```cpp
rkllm_run(llmHandle, &rkllm_input, &rkllm_infer_params, NULL);
```
