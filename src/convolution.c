#include "convolution.h"

const float box_blur_3x3[9] = {1.0f / 9, 1.0f / 9, 1.0f / 9, 1.0f / 9, 1.0f / 9,
                               1.0f / 9, 1.0f / 9, 1.0f / 9, 1.0f / 9};

const float identity_3x3[9] = {0, 0, 0, 0, 1, 0, 0, 0, 0};

const float sobel_x_3x3[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};

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
            float pixel = get_pixel_border(input_image, img_x, img_y, c, width,
                                           height, border_mode);
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

/* Перевод результата свёртки в 8-битное изображение: округление до
 * ближайшего целого и обрезка в диапазон [0, 255] */
void convert_to_bytes(const float *values, int count,
                      int take_absolute_value, unsigned char *output) {
    for (int i = 0; i < count; ++i) {
        float value = values[i];
        if (take_absolute_value && value < 0.0f) {
            value = -value;
        }
        int rounded_value = (value >= 0.0f) ? (int)(value + 0.5f)
                                            : (int)(value - 0.5f);
        if (rounded_value < 0) {
            rounded_value = 0;
        }
        if (rounded_value > 255) {
            rounded_value = 255;
        }
        output[i] = (unsigned char)rounded_value;
    }
}
