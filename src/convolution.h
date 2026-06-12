#ifndef CONVOLUTION_H
#define CONVOLUTION_H

typedef enum {
    BORDER_REFLECT101,
    BORDER_REFLECT,
    BORDER_REPLICATE,
    BORDER_CONSTANT
} border_mode_t;

/* Ядра определены один раз в convolution.c, а здесь только extern-объявления.
Это стандартный способ для C, при котором массивы не дублируются в памяти, а существуют в одном экземпляре.
Есть альтернативный способ определения сразу в заголовочном файле через static const, но тогда каждый .c файл
получит копию массива и компилятор может выдавать предупреждения там, где какое-то ядро не используется.
Просто const-массивы дают ошибку линковки */
extern const float box_blur_3x3[9];
extern const float identity_3x3[9];
extern const float sobel_x_3x3[9];

int convolve_rgb(const unsigned char *input_image, int width, int height,
                 const float *kernel, int kernel_size, int take_absolute_value,
                 unsigned char *output_image, border_mode_t border_mode);

void convert_to_bytes(const float *values, int count,
                      int take_absolute_value, unsigned char *output);

#endif
