/*
 * DeepSeek-R1-Distill-Qwen-1.5B RKLLM Demo
 * 基于正点原子示例修改
 */

#include <string.h>
#include <unistd.h>
#include <string>
#include "rkllm.h"
#include <fstream>
#include <iostream>
#include <csignal>
#include <vector>

using namespace std;
LLMHandle llmHandle = nullptr;

// DeepSeek-R1-Distill 对话模板
#define PROMPT_TEXT_PREFIX "<｜begin▁of▁sentence｜><｜User｜>"
#define PROMPT_TEXT_POSTFIX "<｜Assistant｜>"

void exit_handler(int signal)
{
    if (llmHandle != nullptr)
    {
        cout << "程序即将退出" << endl;
        LLMHandle _tmp = llmHandle;
        llmHandle = nullptr;
        rkllm_destroy(_tmp);
    }
    exit(signal);
}

// 回调函数，处理模型输出
void callback(RKLLMResult *result, void *userdata, LLMCallState state)
{
    if (state == RKLLM_RUN_FINISH)
    {
        printf("\n");
    }
    else if (state == RKLLM_RUN_ERROR)
    {
        printf("\\run error\n");
    }
    else if (state == RKLLM_RUN_NORMAL)
    {
        if (result->text != NULL)
        {
            printf("%s", result->text);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " model_path max_new_tokens max_context_len" << std::endl;
        return 1;
    }

    signal(SIGINT, exit_handler);
    printf("rkllm init start\n");

    // 设置参数及初始化
    RKLLMParam param = rkllm_createDefaultParam();
    param.model_path = argv[1];

    // 设置采样参数
    param.top_k = 1;
    param.top_p = 0.95;
    param.temperature = 0.8;
    param.repeat_penalty = 1.1;
    param.frequency_penalty = 0.0;
    param.presence_penalty = 0.0;

    param.max_new_tokens = std::atoi(argv[2]);
    param.max_context_len = std::atoi(argv[3]);
    param.skip_special_token = true;
    param.extend_param.base_domain_id = 0;

    int ret = rkllm_init(&llmHandle, &param, callback);
    if (ret == 0)
    {
        printf("rkllm init success\n");
    }
    else
    {
        printf("rkllm init failed\n");
        exit_handler(-1);
    }

    // 预设问题
    vector<string> pre_input;
    pre_input.push_back("现有一笼子，里面有鸡和兔子若干只，数一数，共有头14个，腿38条，求鸡和兔子各有多少只？");
    pre_input.push_back("有28位小朋友排成一行,从左边开始数第10位是学豆,从右边开始数他是第几位?");

    cout << "\n**********************可输入以下问题对应序号获取回答/或自定义输入********************\n"
         << endl;
    for (int i = 0; i < (int)pre_input.size(); i++)
    {
        cout << "[" << i << "] " << pre_input[i] << endl;
    }
    cout << "\n*************************************************************************\n"
         << endl;

    // 初始化输入结构体
    RKLLMInput rkllm_input;
    memset(&rkllm_input, 0, sizeof(RKLLMInput));

    // 初始化推理参数结构体
    RKLLMInferParam rkllm_infer_params;
    memset(&rkllm_infer_params, 0, sizeof(RKLLMInferParam));
    rkllm_infer_params.mode = RKLLM_INFER_GENERATE;

    // 主循环
    while (true)
    {
        std::string input_str;
        printf("\n");
        printf("user: ");
        std::getline(std::cin, input_str);

        if (input_str == "exit")
        {
            break;
        }

        // 检查是否是预设问题
        for (int i = 0; i < (int)pre_input.size(); i++)
        {
            if (input_str == to_string(i))
            {
                input_str = pre_input[i];
                cout << input_str << endl;
            }
        }

        // 拼接 prompt
        string text = PROMPT_TEXT_PREFIX + input_str + PROMPT_TEXT_POSTFIX;
        rkllm_input.input_type = RKLLM_INPUT_PROMPT;
        rkllm_input.prompt_input = (char *)text.c_str();
        printf("robot: ");

        // 运行推理
        rkllm_run(llmHandle, &rkllm_input, &rkllm_infer_params, NULL);
    }

    rkllm_destroy(llmHandle);
    return 0;
}
