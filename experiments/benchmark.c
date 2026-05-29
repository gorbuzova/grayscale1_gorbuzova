#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb_image.h"

typedef enum {
    BORDER_REFLECT101,
    BORDER_REFLECT,
    BORDER_REPLICATE,
    BORDER_CONSTANT
} border_mode_t;

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
    if (border_mode == BORDER_CONSTANT) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return 0.0f;
        }
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
            float pixel = get_pixel_border(input_image, img_x, img_y, c, width,
                                           height, border_mode);
            int flipped_kx = kernel_size - 1 - kx;
            int flipped_ky = kernel_size - 1 - ky;
            sum += pixel * kernel[flipped_ky * kernel_size + flipped_kx];
        }
    }
    return sum;
}

static void convolve_rgb(const float *input_image, int width, int height,
                         const float *kernel, int kernel_size,
                         float *output_image, border_mode_t border_mode) {
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

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_image> <output_measurements_file>\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    const char *measurements_file = argv[2];

    const int number_of_warmup_runs = 3;
    const int number_of_measurements = 40;
    const int kernel_size = 3;

    int width, height, channels;
    unsigned char *image_bytes =
        stbi_load(input_file, &width, &height, &channels, 3);
    if (!image_bytes) {
        printf("Failed to load image: %s\n", input_file);
        return 1;
    }
    printf("Image loaded: %d x %d, RGB\n", width, height);

    int number_of_values = width * height * 3;
    float *input_image = (float *)malloc(number_of_values * sizeof(float));
    float *output_image = (float *)malloc(number_of_values * sizeof(float));
    if (!input_image || !output_image) {
        printf("Memory allocation failed.\n");
        free(input_image);
        free(output_image);
        stbi_image_free(image_bytes);
        return 1;
    }

    for (int index = 0; index < number_of_values; ++index) {
        input_image[index] = image_bytes[index];
    }
    stbi_image_free(image_bytes);

    FILE *output = fopen(measurements_file, "w");
    if (!output) {
        printf("Failed to open output file: %s\n", measurements_file);
        free(input_image);
        free(output_image);
        return 1;
    }

    /* Прогревочные запуски. Самые первые запуски обычно медленнее из-за
     "холодного" кеша процессора, поэтому их результаты не записываем */
    for (int run = 0; run < number_of_warmup_runs; ++run) {
        convolve_rgb(input_image, width, height, box_blur_3x3, kernel_size,
                     output_image, BORDER_REFLECT101);
    }

    for (int run = 0; run < number_of_measurements; ++run) {
        struct timespec start_time;
        struct timespec end_time;

        clock_gettime(CLOCK_MONOTONIC, &start_time);
        convolve_rgb(input_image, width, height, box_blur_3x3, kernel_size,
                     output_image, BORDER_REFLECT101);
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        double seconds = (double)(end_time.tv_sec - start_time.tv_sec);
        double nanoseconds = (double)(end_time.tv_nsec - start_time.tv_nsec);
        double milliseconds = seconds * 1000.0 + nanoseconds / 1000000.0;

        fprintf(output, "%.6f\n", milliseconds);
        printf("Measurement %d: %.6f ms\n", run + 1, milliseconds);
    }

    fclose(output);
    free(input_image);
    free(output_image);

    printf("Saved %d measurements to %s\n", number_of_measurements,
           measurements_file);
    return 0;
}
