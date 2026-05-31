#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/convolution.h"

static float box_blur_3x3[9] = {1.0f / 9, 1.0f / 9, 1.0f / 9,
                                1.0f / 9, 1.0f / 9, 1.0f / 9,
                                1.0f / 9, 1.0f / 9, 1.0f / 9};

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <width> <height> <output_measurements_file>\n",
               argv[0]);
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    const char *measurements_file = argv[3];

    if (width <= 0 || height <= 0) {
        printf("Width and height must be positive numbers.\n");
        return 1;
    }

    const int number_of_warmup_runs = 3;
    const int number_of_measurements = 40;
    const int kernel_size = 3;

    int number_of_values = width * height * 3;
    float *input_image = (float *)malloc(number_of_values * sizeof(float));
    float *output_image = (float *)malloc(number_of_values * sizeof(float));
    if (!input_image || !output_image) {
        printf("Memory allocation failed.\n");
        free(input_image);
        free(output_image);
        return 1;
    }

    /* Заполняем изображение случайными значениями от 0 до 255. */
    srand(12345);
    for (int index = 0; index < number_of_values; ++index) {
        input_image[index] = (float)(rand() % 256);
    }
    printf("Random image created: %d x %d, RGB\n", width, height);

    FILE *output = fopen(measurements_file, "w");
    if (!output) {
        printf("Failed to open output file: %s\n", measurements_file);
        free(input_image);
        free(output_image);
        return 1;
    }

    /* Прогревочные запуски: первые запуски обычно медленнее из-за "холодного"
     * кеша процессора, поэтому их результаты не записываем. */
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
