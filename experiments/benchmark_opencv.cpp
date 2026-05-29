#include <chrono>
#include <cstdio>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_image> <output_measurements_file>\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    const char *measurements_file = argv[2];
    const int number_of_warmup_runs = 3;
    const int number_of_measurements = 40;

    // Моя реализация однопоточная, поэтому для честного сравнения запрещаем
    // OpenCV использовать несколько потоков
    cv::setNumThreads(1);
    cv::Mat image = cv::imread(input_file, cv::IMREAD_COLOR);
    if (image.empty()) {
        printf("Could not read image: %s\n", input_file);
        return 1;
    }
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    printf("Image loaded: %d x %d, RGB\n", image.cols, image.rows);

    cv::Mat kernel_box_blur = cv::Mat::ones(3, 3, CV_32F) / 9.0f;
    cv::Mat output_image;

    FILE *output = fopen(measurements_file, "w");
    if (!output) {
        printf("Failed to open output file: %s\n", measurements_file);
        return 1;
    }

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
        fprintf(output, "%.6f\n", milliseconds);
        printf("Measurement %d: %.6f ms\n", run + 1, milliseconds);
    }

    fclose(output);
    printf("Saved %d measurements to %s\n", number_of_measurements,
           measurements_file);
    return 0;
}
