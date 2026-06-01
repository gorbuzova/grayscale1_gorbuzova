#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/convolution.h"

static float box_blur_3x3[9] = {1.0f / 9, 1.0f / 9, 1.0f / 9,
                                1.0f / 9, 1.0f / 9, 1.0f / 9,
                                1.0f / 9, 1.0f / 9, 1.0f / 9};

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <output_measurements_file>\n", argv[0]);
        return 1;
    }

    const char *measurements_file = argv[1];

    const int number_of_warmup_runs = 3;
    const int number_of_measurements = 40;
    const int kernel_size = 3;

    /* Размеры изображений для эксперимента (ширина и высота) */
    const int widths[3] = {1280, 1920, 2560};
    const int heights[3] = {960, 1440, 1920};
    const int number_of_sizes = 3;

    FILE *output = fopen(measurements_file, "w");
    if (!output) {
        printf("Failed to open output file: %s\n", measurements_file);
        return 1;
    }

    for (int size_index = 0; size_index < number_of_sizes; ++size_index) {
        const int width = widths[size_index];
        const int height = heights[size_index];

        int number_of_values = width * height * 3;
        float *input_image = (float *)malloc(number_of_values * sizeof(float));
        float *output_image = (float *)malloc(number_of_values * sizeof(float));
        if (!input_image || !output_image) {
            printf("Memory allocation failed.\n");
            free(input_image);
            free(output_image);
            fclose(output);
            return 1;
        }

        /* Заполняем изображение случайными значениями от 0 до 255 */
        srand(12345);
        for (int index = 0; index < number_of_values; ++index) {
            input_image[index] = (float)(rand() % 256);
        }
        printf("Random image created: %d x %d, RGB\n", width, height);

        /* Прогревочные запуски: первые запуски обычно медленнее из-за
         * "холодного" кеша процессора, поэтому их результаты не записываем. */
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

            fprintf(output, "%d %d %.6f\n", width, height, milliseconds);
            printf("Size %d x %d, measurement %d: %.6f ms\n", width, height,
                   run + 1, milliseconds);
        }

        free(input_image);
        free(output_image);
    }

    fclose(output);
    printf("Saved measurements to %s\n", measurements_file);
    return 0;
}
