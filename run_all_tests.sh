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

border_modes=(reflect101 reflect replicate constant)
for IMAGE in "${images[@]}"; do
    if [ ! -f "$IMAGE" ]; then
        continue
    fi

    BASENAME=$(basename "$IMAGE")
    NAME="${BASENAME%.*}"

    for BORDER_MODE in "${border_modes[@]}"; do
        echo "Testing image: $IMAGE, border mode: $BORDER_MODE"

        echo "- Generating OpenCV reference -"
        ./build/opencv_reference "$IMAGE" "$BORDER_MODE"

        echo "- Running box blur -"
        ./build/convolution "$IMAGE" "box3_out_${NAME}_${BORDER_MODE}.png" box "$BORDER_MODE"

        echo "- Running identity -"
        ./build/convolution "$IMAGE" "identity_out_${NAME}_${BORDER_MODE}.png" identity "$BORDER_MODE"

        echo "- Running sobelx -"
        ./build/convolution "$IMAGE" "sobelx_out_${NAME}_${BORDER_MODE}.png" sobelx "$BORDER_MODE"

        echo "- Comparing results -"
        ./build/test_compare blur_result.png "box3_out_${NAME}_${BORDER_MODE}.png"
        ./build/test_compare identity_result.png "identity_out_${NAME}_${BORDER_MODE}.png"
        ./build/test_compare sobel_result.png "sobelx_out_${NAME}_${BORDER_MODE}.png"
    done
done


echo "All tests completed"

