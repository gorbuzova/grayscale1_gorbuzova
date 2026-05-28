#include <cstring>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../src/stb_image_write.h"
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        std::cout << "Usage: " << argv[0] << " <image_path> [border_mode]"
                  << std::endl;
        return -1;
    }
    const char *border_mode_name = "reflect101";
    if (argc == 3) {
        border_mode_name = argv[2];
    }
    int border_type = cv::BORDER_REFLECT_101;
    if (std::strcmp(border_mode_name, "reflect") == 0) {
        border_type = cv::BORDER_REFLECT;
    } else if (std::strcmp(border_mode_name, "replicate") == 0) {
        border_type = cv::BORDER_REPLICATE;
    } else if (std::strcmp(border_mode_name, "constant") == 0) {
        border_type = cv::BORDER_CONSTANT;
    } else if (std::strcmp(border_mode_name, "reflect101") != 0) {
        std::cout << "Unknown border mode: " << border_mode_name << std::endl;
        return -1;
    }

    cv::Mat img = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cout << "Could not read image: " << argv[1] << std::endl;
        return -1;
    }
    cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

    cv::Mat kernel_blur = cv::Mat::ones(3, 3, CV_32F) / 9.0f;
    cv::Mat kernel_identity = cv::Mat::zeros(3, 3, CV_32F);
    kernel_identity.at<float>(1, 1) = 1.0f;
    cv::Mat kernel_sobel_x =
        (cv::Mat_<float>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);

    cv::Mat blur_result, identity_result, sobel_result;
    cv::filter2D(img, blur_result, CV_32F, kernel_blur, cv::Point(-1, -1), 0,
                 border_type);
    cv::filter2D(img, identity_result, CV_32F, kernel_identity,
                 cv::Point(-1, -1), 0, border_type);
    cv::Mat kernel_sobel_x_flipped;
    cv::flip(kernel_sobel_x, kernel_sobel_x_flipped, -1);
    cv::filter2D(img, sobel_result, CV_32F, kernel_sobel_x_flipped,
                 cv::Point(-1, -1), 0, border_type);

    cv::Mat blur_show, identity_show, sobel_show;
    blur_result.convertTo(blur_show, CV_8U);
    identity_result.convertTo(identity_show, CV_8U);
    cv::convertScaleAbs(sobel_result, sobel_show);
    cv::Mat blur_bgr, identity_bgr, sobel_bgr;
    cv::cvtColor(blur_show, blur_bgr, cv::COLOR_RGB2BGR);
    cv::cvtColor(identity_show, identity_bgr, cv::COLOR_RGB2BGR);
    cv::cvtColor(sobel_show, sobel_bgr, cv::COLOR_RGB2BGR);
    cv::imwrite("blur_result.png", blur_bgr);
    cv::imwrite("identity_result.png", identity_bgr);
    cv::imwrite("sobel_result.png", sobel_bgr);

    std::cout << "Results saved:\n";
    std::cout << "  Box blur: blur_result.png\n";
    std::cout << "  Identity: identity_result.png\n";
    std::cout << "  Sobel X: sobel_result.png\n";

    return 0;
}
