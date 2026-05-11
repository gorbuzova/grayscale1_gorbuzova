#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../src/stb_image_write.h"
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return -1;
    }

    int width, height, channels;
    unsigned char *img_data = stbi_load(argv[1], &width, &height, &channels, 3);
    if (!img_data) {
        std::cout << "Could not read image: " << argv[1] << std::endl;
        return -1;
    }
    cv::Mat img(height, width, CV_8UC3, img_data);
    channels = 3;

    cv::Mat kernel_blur = cv::Mat::ones(3, 3, CV_32F) / 9.0f;
    cv::Mat kernel_identity = cv::Mat::zeros(3, 3, CV_32F);
    kernel_identity.at<float>(1, 1) = 1.0f;
    cv::Mat kernel_sobel_x =
        (cv::Mat_<float>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);

    cv::Mat blur_result, identity_result, sobel_result;
    cv::filter2D(img, blur_result, CV_32F, kernel_blur);
    cv::filter2D(img, identity_result, CV_32F, kernel_identity);
    cv::Mat kernel_sobel_x_flipped;
    cv::flip(kernel_sobel_x, kernel_sobel_x_flipped, -1);
    cv::filter2D(img, sobel_result, CV_32F, kernel_sobel_x_flipped);
    cv::Mat blur_show, identity_show, sobel_show;
    blur_result.convertTo(blur_show, CV_8U);
    identity_result.convertTo(identity_show, CV_8U);
    cv::convertScaleAbs(sobel_result, sobel_show);

    stbi_write_png("blur_result.png",
                   blur_show.cols,
                   blur_show.rows,
                   blur_show.channels(),
                   blur_show.data,
                   blur_show.step);
    stbi_write_png("identity_result.png",
                   identity_show.cols,
                   identity_show.rows,
                   identity_show.channels(),
                   identity_show.data,
                   identity_show.step);
    stbi_write_png("sobel_result.png",
                   sobel_show.cols,
                   sobel_show.rows,
                   sobel_show.channels(),
                   sobel_show.data,
                   sobel_show.step);

    std::cout << "Results saved:\n";
    std::cout << "  Box blur: blur_result.png\n";
    std::cout << "  Identity: identity_result.png\n";
    std::cout << "  Sobel X: sobel_result.png\n";

    return 0;
}
