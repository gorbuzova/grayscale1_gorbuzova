#!/bin/bash
set -e

# Проверяем, передан ли путь к изображению
if [ $# -ne 1 ]; then
    echo "Usage: $0 <image_path>"
    exit 1
fi

IMAGE="$1"

# Генерация эталонов OpenCV
echo "- Generating OpenCV reference -"
./tests/opencv_reference "$IMAGE"

# Свёртка для трёх ядер
echo "- Running box blur -"
./src/grayscale "$IMAGE" "box3_out.png" 3 box

echo "- Running identity -"
./src/grayscale "$IMAGE" "identity_out.png" 3 identity

echo "- Running sobelx -"
./src/grayscale "$IMAGE" "sobelx_out.png" 3 sobelx

# Сравнение
echo "- Comparing results -"
./tests/test_compare blur_result.png box3_out.png
./tests/test_compare identity_result.png identity_out.png
./tests/test_compare sobel_result.png sobelx_out.png

echo "All tests completed"
