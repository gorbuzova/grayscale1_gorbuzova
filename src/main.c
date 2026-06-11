#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "convolution.h"

int main(int argc, char **argv) {
    unsigned char *img_data = NULL;
    float *input_float = NULL;
    float *output_float = NULL;
    unsigned char *result_bytes = NULL;
    int status = 1;

    if (argc < 3 || argc > 5) {
        fprintf(stderr,
                "Usage: %s <input_image> <output_image> [kernel_type] "
                "[border_mode]\n",
                argv[0]);
        fprintf(stderr, "kernel_type: box (default), identity, sobelx\n");
        fprintf(stderr, "border_mode: reflect101 (default), reflect, "
                        "replicate, constant\n");
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const int kernel_size = 3;

    const char *kernel_type = "box";
    if (argc >= 4) {
        kernel_type = argv[3];
    }
    const char *border_mode_name = "reflect101";
    if (argc == 5) {
        border_mode_name = argv[4];
    }
    border_mode_t border_mode;
    if (strcmp(border_mode_name, "reflect101") == 0) {
        border_mode = BORDER_REFLECT101;
    } else if (strcmp(border_mode_name, "reflect") == 0) {
        border_mode = BORDER_REFLECT;
    } else if (strcmp(border_mode_name, "replicate") == 0) {
        border_mode = BORDER_REPLICATE;
    } else if (strcmp(border_mode_name, "constant") == 0) {
        border_mode = BORDER_CONSTANT;
    } else {
        fprintf(stderr, "Unknown border mode: %s\n", border_mode_name);
        goto cleanup;
    }
    /* Загрузка изображения как RGB */
    int width, height, channels;
    img_data = stbi_load(input_file, &width, &height, &channels, 3);
    if (!img_data) {
        fprintf(stderr, "Failed to load image: %s\n", input_file);
        goto cleanup;
    }

    printf("Image loaded: %d x %d, RGB\n", width, height);

    int num_values = width * height * 3;
    input_float = (float *)malloc(num_values * sizeof(float));
    output_float = (float *)malloc(num_values * sizeof(float));
    if (!input_float || !output_float) {
        fprintf(stderr, "Memory allocation failed.\n");
        goto cleanup;
    }

    for (int i = 0; i < num_values; ++i) {
        input_float[i] = img_data[i];
    }
    stbi_image_free(img_data);
    img_data = NULL;

    /* Выбор ядра */
    const int is_sobelx = strcmp(kernel_type, "sobelx") == 0;

    const float *kernel;
    if (strcmp(kernel_type, "identity") == 0) {
        kernel = identity_3x3;
    } else if (is_sobelx) {
        kernel = sobel_x_3x3;
    } else {
        kernel = box_blur_3x3;
    }

    /* Свёртка */
    convolve_rgb(input_float, width, height, kernel, kernel_size, output_float,
                 border_mode);

    /* Конвертация обратно в 8 бит */
    result_bytes = (unsigned char *)malloc(num_values);
    if (!result_bytes) {
        fprintf(stderr, "Memory allocation failed.\n");
        goto cleanup;
    }
    convert_to_bytes(output_float, num_values, is_sobelx, result_bytes);

    /* Сохранение изображения в формате PNG */
    int write_ok =
        stbi_write_png(output_file, width, height, 3, result_bytes, width * 3);
    if (!write_ok) {
        fprintf(stderr, "Failed to write output image: %s\n", output_file);
        goto cleanup;
    } else {
        printf("Output saved to %s\n", output_file);
        status = 0;
    }

cleanup:
    stbi_image_free(img_data);
    free(input_float);
    free(output_float);
    free(result_bytes);
    return status;
}
