#ifndef CONVOLUTION_H
#define CONVOLUTION_H

typedef enum {
    BORDER_REFLECT101,
    BORDER_REFLECT,
    BORDER_REPLICATE,
    BORDER_CONSTANT
} border_mode_t;

void convolve_rgb(const float *input_image, int width, int height,
                  const float *kernel, int kernel_size, float *output_image,
                  border_mode_t border_mode);

#endif
