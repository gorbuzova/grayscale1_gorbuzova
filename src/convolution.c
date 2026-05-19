#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef enum {
    BORDER_REFLECT101,
    BORDER_REFLECT,
    BORDER_REPLICATE,
    BORDER_CONSTANT
} border_mode_t;
/* Метод зеркального отражения относительно границы (без дублирования крайнего
пикселя). Аналогичен BORDER_REFLECT_101 в OpenCV. Предпочтителен для градиентных
фильтров, например Sobel. */
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

/* Метод отражения с повторением крайнего пикселя, аналог BORDER_REFLECT в
 * OpenCV */
static int reflect(int position, int size) {
    if (size == 1) {
        return 0;
    }
    while (position < 0 || position >= size) {
        if (position < 0) {
            position = -position - 1;
        } else {
            position = 2 * size - position - 1;
        }
    }
    return position;
}

static int clamp_int(int value, int min_value, int max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}
static float get_pixel_border(const float *image_data, int x, int y, int c,
                              int width, int height,
                              border_mode_t border_mode) {
    /* за границей подставляется одно и то же значение (0) */
    if (border_mode == BORDER_CONSTANT) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return 0.0f;
        }
        /* повторяется ближайший пиксель */
    } else if (border_mode == BORDER_REPLICATE) {
        x = clamp_int(x, 0, width - 1);
        y = clamp_int(y, 0, height - 1);
    } else if (border_mode == BORDER_REFLECT) {
        x = reflect(x, width);
        y = reflect(y, height);
    } else {
        x = reflect101(x, width);
        y = reflect101(y, height);
    }
    return image_data[(y * width + x) * 3 + c];
}

static float convolve_at_pixel(const float *input_image, int x, int y, int c,
                               int width, int height, const float *kernel,
                               int kernel_size, border_mode_t border_mode) {
    int radius = kernel_size / 2;
    float sum = 0.0f;
    for (int ky = 0; ky < kernel_size; ++ky) {
        for (int kx = 0; kx < kernel_size; ++kx) {
            int img_x = x + (kx - radius);
            int img_y = y + (ky - radius);
            float pixel = get_pixel_border(input_image, img_x, img_y, c,
                                           width, height, border_mode);
            int flipped_kx = kernel_size - 1 - kx;
            int flipped_ky = kernel_size - 1 - ky;
            sum += pixel * kernel[flipped_ky * kernel_size + flipped_kx];
        }
    }
    return sum;
}

void convolve_rgb(const float *input_image, int width, int height,
                  const float *kernel, int kernel_size, float *output_image,
                  border_mode_t border_mode) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < 3; ++c) {
                output_image[(y * width + x) * 3 + c] =
                    convolve_at_pixel(input_image, x, y, c, width, height,
                                      kernel, kernel_size, border_mode);
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
    // Загрузка изображения как RGB
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
    convolve_rgb(input_float, width, height, kernel, kernel_size, output_float,
                 border_mode);

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
