#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/* Метод зеркального отражения относительно границы.
Аналогичен BORDER_REFLECT_101 в OpenCV.
Предпочтителен для градиентных фильтров, например Sobel. */
static int reflect101(int position, int size) {
    if (size == 1) {
        return 0;
    }

    while (position < 0 || position >= size) {
        if (position < 0) {
            position = -position;
        } else {
            position = 2 * size - position - 2;
        }
    }
    return position;
}

static float get_pixel_reflect(const float *image_data, int x, int y, int c,
                               int width, int height) {
    x = reflect101(x, width);
    y = reflect101(y, height);
    return image_data[(y * width + x) * 3 + c];
}

void convolve_rgb(const float *input_image, int width, int height,
                  const float *kernel, int kernel_size,
                  float *output_image) {
    int radius = kernel_size / 2;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < 3; ++c) {
                float sum = 0.0f;
                for (int ky = 0; ky < kernel_size; ++ky) {
                    for (int kx = 0; kx < kernel_size; ++kx) {
                        int img_x = x + (kx - radius);
                        int img_y = y + (ky - radius);
                        float pixel = get_pixel_reflect(
                            input_image, img_x, img_y, c, width, height);
                        int flipped_kx = kernel_size - 1 - kx;
                        int flipped_ky = kernel_size - 1 - ky;
                        sum += pixel *
                               kernel[flipped_ky * kernel_size + flipped_kx];
                    }
                }
                output_image[(y * width + x) * 3 + c] = sum;
            }
        }
    }
}

static float box_blur_3x3[9] = {1.0f / 9, 1.0f / 9, 1.0f / 9,
                                1.0f / 9, 1.0f / 9, 1.0f / 9,
                                1.0f / 9, 1.0f / 9, 1.0f / 9};

static float identity_3x3[9] = {0, 0, 0, 0, 1, 0, 0, 0, 0};

static float sobel_x_3x3[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};

int main(int argc, char **argv) {
    unsigned char *img_data = NULL;
    float *input_float = NULL;
    float *output_float = NULL;
    float *kernel = NULL;
    unsigned char *result_bytes = NULL;
    int status = 1;

    if (argc < 3 || argc > 4) {
        fprintf(stderr,
                "Usage: %s <input_image> <output_image> [kernel_type]\n",
                argv[0]);
        fprintf(stderr, "kernel_type: box (default), identity, sobelx\n");
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const int kernel_size = 3;

    const char *kernel_type = "box";
    if (argc == 4) {
        kernel_type = argv[3];
    }

    // Загрузка изображения как RGB
    int width, height, channels;
    img_data = stbi_load(input_file, &width, &height, &channels, 3);
    if (!img_data) {
        fprintf(stderr, "Failed to load image: %s\n", input_file);
        goto cleanup;
    }
    channels = 3;

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

    // Создаём ядро
    kernel = (float *)malloc(kernel_size * kernel_size * sizeof(float));
    if (!kernel) {
        fprintf(stderr, "Memory allocation failed.\n");
        goto cleanup;
    }

    const int is_identity = strcmp(kernel_type, "identity") == 0;
    const int is_sobelx = strcmp(kernel_type, "sobelx") == 0;

    if (is_identity) {
        memcpy(kernel, identity_3x3, sizeof(identity_3x3));
    } else if (is_sobelx) {
        memcpy(kernel, sobel_x_3x3, sizeof(sobel_x_3x3));
    } else {
        memcpy(kernel, box_blur_3x3, sizeof(box_blur_3x3));
    }

    // Свёртка
    convolve_rgb(input_float, width, height, kernel, kernel_size,
                       output_float);

    // Конвертация обратно в 8 бит
    result_bytes = (unsigned char *)malloc(num_values);
    for (int i = 0; i < num_values; ++i) {
        float pixel_value = output_float[i];
        if (is_sobelx) {
            pixel_value = fabsf(pixel_value);
        }
        int clipped_int = (pixel_value >= 0) ? (int)(pixel_value + 0.5f)
                                             : (int)(pixel_value - 0.5f);
        if (clipped_int < 0) {
            clipped_int = 0;
        }
        if (clipped_int > 255) {
            clipped_int = 255;
        }
        result_bytes[i] = (unsigned char)clipped_int;
    }
    // Сохранение изображение в формате PNG
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
    free(kernel);
    free(result_bytes);
    return status;
}
