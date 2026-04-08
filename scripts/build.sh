#!/bin/bash
set -e  # Остановить скрипт при любой ошибке

sudo apt update
sudo apt install -y libopencv-dev g++ gcc pkg-config

# Сборка программы
cd src
gcc grayscale.c -o grayscale -lm
cd ..

# Сборка OpenCV-примера
cd tests
g++ opencv_reference.cpp -o opencv_reference -I../src `pkg-config --cflags --libs opencv4`
cd ..

# Сборка программы сравнения test_compare
cd tests
gcc test.c -o test_compare -I../src -lm
cd ..

echo "Build completed"
