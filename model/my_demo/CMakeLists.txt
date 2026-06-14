cmake_minimum_required(VERSION 3.8)
project(my_deepseek_demo)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/lib/)

# 链接库文件
set(RKLLM_RT_LIB ${CMAKE_SOURCE_DIR}/lib/librkllmrt.so)

# 添加可执行文件
add_executable(my_demo src/main.cpp)
target_link_libraries(my_demo ${RKLLM_RT_LIB})

# 安装配置
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)
install(TARGETS my_demo DESTINATION ./)
