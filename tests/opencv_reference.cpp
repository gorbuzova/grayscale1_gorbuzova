#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return -1;
    }

    cv::Mat img_color = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (img_color.empty()) {
        std::cout << "Could not read image: " << argv[1] << std::endl;
        return -1;
    }

    cv::Mat img;
    cv::cvtColor(img_color, img, cv::COLOR_BGR2RGB);

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

    cv::imwrite("blur_result.png", blur_show);
    cv::imwrite("identity_result.png", identity_show);
    cv::imwrite("sobel_result.png", sobel_show);

    std::cout << "Results saved:\n";
    std::cout << "  Box blur: blur_result.png\n";
    std::cout << "  Identity: identity_result.png\n";
    std::cout << "  Sobel X: sobel_result.png\n";

    return 0;
}
