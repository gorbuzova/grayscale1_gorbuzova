# Свёртка RGB-изображений

Реализована двумерная свёртка для RGB-изображений с ядрами box blur, identity и Sobel X. Поддерживаются 4 режима обработки границ: reflect101, reflect, replicate и constant (по умолчанию - reflect101). Корректность проверяется сравнением с OpenCV.

## Сборка

```bash
mkdir build
cd build
cmake ..
make
cd ..
```

## Запуск свёртки (из корневой директории)

```bash
./build/convolution <input_image> <output_image> [kernel_type] [border_mode]
```

Примеры:

```bash
./build/convolution images/rose.jpg out.png
./build/convolution images/rose.jpg out.png box reflect101
./build/convolution images/rose.jpg out.png box reflect
./build/convolution images/rose.jpg out.png identity replicate
./build/convolution images/rose.jpg out.png sobelx constant
```

По умолчанию используется box.

## Запуск тестов

```bash
chmod +x run_all_tests.sh
./run_all_tests.sh
```

Скрипт автоматически прогоняет все изображения из папки images и сравнивает результат с OpenCV, тесты запускаются для всех четырёх способов обработки краёв.

# Пример добавления своего изображения

```bash
cp ~/Downloads/your_image.jpg images/
```

## Эксперименты (замеры производительности)

Сравнение скорости моей реализации с OpenCV. Оба бенчмарка работают на
случайных изображениях трёх размеров (заданы в
`experiments/benchmark_config.h`), делают прогревочные запуски и по 40 замеров
на каждый размер. OpenCV принудительно переведён в один поток, а перевод
результата в 8 бит включён в измеряемое время в обеих реализациях - чтобы
сравнивать одинаковый объём работы.

Сборка происходит вместе с остальными целями (см. раздел «Сборка»).

Запуск бенчмарков из корня:

./build/benchmark my_measurements.txt
./build/benchmark_opencv opencv_measurements.txt

Каждый пишет в файл строки вида `ширина высота время_мс`.

Анализ (нужны numpy, scipy, matplotlib - `pip install numpy scipy matplotlib`):

python experiments/experiment_analysis.py my_measurements.txt opencv_measurements.txt

По каждому размеру скрипт строит гистограмму, проверяет нормальность
(normaltest и shapiro), считает среднее, стандартное отклонение и
доверительный интервал 95%, округляет результат по правилам значащих цифр и
сравнивает реализации через отношение средних времён с распространением
погрешности. Дополнительно сохраняются графики speedup.png и comparison.png.
