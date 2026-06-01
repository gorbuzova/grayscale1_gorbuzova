#include <chrono>
#include <cstdio>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <output_measurements_file>\n", argv[0]);
        return 1;
    }

    const char *measurements_file = argv[1];

    const int number_of_warmup_runs = 3;
    const int number_of_measurements = 40;

    // Размеры изображений для эксперимента (ширина и высота)
    const int widths[3] = {1280, 1920, 2560};
    const int heights[3] = {960, 1440, 1920};
    const int number_of_sizes = 3;

    // Моя реализация однопоточная, поэтому для честного сравнения запрещаем
    // OpenCV использовать несколько потоков
    cv::setNumThreads(1);

    cv::Mat kernel_box_blur = cv::Mat::ones(3, 3, CV_32F) / 9.0f;
    cv::Mat output_image;

    FILE *output = fopen(measurements_file, "w");
    if (!output) {
        printf("Failed to open output file: %s\n", measurements_file);
        return 1;
    }

    for (int size_index = 0; size_index < number_of_sizes; ++size_index) {
        const int width = widths[size_index];
        const int height = heights[size_index];

        // Создаём случайное изображение
        cv::setRNGSeed(12345);
        cv::Mat image(height, width, CV_8UC3);
        cv::randu(image, cv::Scalar(0, 0, 0), cv::Scalar(256, 256, 256));
        printf("Random image created: %d x %d, RGB\n", image.cols, image.rows);

        // Прогревочные запуски (их результаты не записываем)
        for (int run = 0; run < number_of_warmup_runs; ++run) {
            cv::filter2D(image, output_image, CV_32F, kernel_box_blur,
                         cv::Point(-1, -1), 0, cv::BORDER_REFLECT_101);
        }

        for (int run = 0; run < number_of_measurements; ++run) {
            std::chrono::steady_clock::time_point start_time =
                std::chrono::steady_clock::now();

            cv::filter2D(image, output_image, CV_32F, kernel_box_blur,
                         cv::Point(-1, -1), 0, cv::BORDER_REFLECT_101);

            std::chrono::steady_clock::time_point end_time =
                std::chrono::steady_clock::now();

            double milliseconds =
                std::chrono::duration<double, std::milli>(end_time - start_time)
                    .count();

            fprintf(output, "%d %d %.6f\n", width, height, milliseconds);
            printf("Size %d x %d, measurement %d: %.6f ms\n", width, height,
                   run + 1, milliseconds);
        }
    }

    fclose(output);
    printf("Saved measurements to %s\n", measurements_file);
    return 0;
}
