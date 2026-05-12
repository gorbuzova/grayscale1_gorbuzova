#define STB_IMAGE_IMPLEMENTATION

#include "../src/stb_image.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <reference_image> <my_image>\n", argv[0]);
        return 1;
    }

    int ref_width, ref_height, ref_channels;
    unsigned char *reference_image =
        stbi_load(argv[1], &ref_width, &ref_height, &ref_channels, 0);
    if (!reference_image) {
        printf("Failed to load reference image: %s\n", argv[1]);
        return 1;
    }

    int my_width, my_height, my_channels;
    unsigned char *my_image =
        stbi_load(argv[2], &my_width, &my_height, &my_channels, 0);
    if (!my_image) {
        printf("Failed to load my image: %s\n", argv[2]);
        stbi_image_free(reference_image);
        return 1;
    }

    if (ref_width != my_width || ref_height != my_height ||
        ref_channels != my_channels) {
        printf("Images have different dimensions or channels: "
               "ref %dx%dx%d, my %dx%dx%d\n",
               ref_width, ref_height, ref_channels, my_width, my_height,
               my_channels);
        stbi_image_free(reference_image);
        stbi_image_free(my_image);
        return 1;
    }

    int total_elements = ref_width * ref_height * ref_channels;
    double max_difference = 0.0;
    for (int i = 0; i < total_elements; i++) {
        double current_diff =
            fabs((double)reference_image[i] - (double)my_image[i]);
        if (current_diff > max_difference)
            max_difference = current_diff;
    }

    printf("Max pixel difference: %f\n", max_difference);
    double allowed_diff = 1.0;

    if (max_difference <= allowed_diff) {
        printf("Images are similar (max diff <= %.1f).\n", allowed_diff);
        stbi_image_free(reference_image);
        stbi_image_free(my_image);
        return 0;
    } else {
        printf("Images differ (max diff > %.1f).\n", allowed_diff);
        stbi_image_free(reference_image);
        stbi_image_free(my_image);
        return 1;
    }
}
