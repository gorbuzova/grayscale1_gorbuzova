#!/bin/bash
set -e
shopt -s nullglob

cmake -S . -B build
cmake --build build

if [ ! -d images ]; then
    echo "images directory not found"
    exit 1
fi

images=(images/*)
if [ ${#images[@]} -eq 0 ]; then
    echo "No images found in images/"
    exit 1
fi

for IMAGE in "${images[@]}"; do
    if [ ! -f "$IMAGE" ]; then
        continue
    fi

    BASENAME=$(basename "$IMAGE")
    NAME="${BASENAME%.*}"

    echo "Testing image: $IMAGE"

    echo "- Generating OpenCV reference -"
    ./build/opencv_reference "$IMAGE"

    echo "- Running box blur -"
    ./build/convolution "$IMAGE" "box3_out_${NAME}.png" box

    echo "- Running identity -"
    ./build/convolution "$IMAGE" "identity_out_${NAME}.png" identity

    echo "- Running sobelx -"
    ./build/convolution "$IMAGE" "sobelx_out_${NAME}.png" sobelx

    echo "- Comparing results -"
    ./build/test_compare blur_result.png "box3_out_${NAME}.png"
    ./build/test_compare identity_result.png "identity_out_${NAME}.png"
    ./build/test_compare sobel_result.png "sobelx_out_${NAME}.png"
done

echo "All tests completed"

