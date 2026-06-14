#ifndef BENCHMARK_CONFIG_H
#define BENCHMARK_CONFIG_H

/* Общие параметры экспериментов для обоих бенчмарков */

static const int number_of_warmup_runs = 3;
static const int number_of_measurements = 40;

/* Размеры изображений для эксперимента (ширина и высота) */
static const int widths[3] = {1280, 1920, 2560};
static const int heights[3] = {960, 1440, 1920};
static const int number_of_sizes = 3;

#endif
