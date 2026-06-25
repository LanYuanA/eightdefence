#!/bin/bash
cd ~/zhangruige/eightdefence
base64 -d b64_hpp.txt > application/apps/fire_fighting/app_fire_fighting.hpp
base64 -d b64_cpp.txt > application/apps/fire_fighting/app_fire_fighting.cpp
base64 -d b64_main.txt > core/main.cpp
base64 -d b64_vue.txt > ui/src/views/FireFighting.vue
echo "All decoded. Run: make clean && make"