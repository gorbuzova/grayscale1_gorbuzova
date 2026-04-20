# Свёртка одноканальных изображений

Реализована двумерная для grayscale-изображений с ядрами box blur, identity и Sobel X. Для обработки границ используется reflect101, корректность проверяется сравнением с OpenCV.

## Сборка

```bash
mkdir build
cd build
cmake ..
make
```

## Запуск свёртки

```bash
./build/grayscale <input_image> <output_image> [kernel_type]
```

Примеры:

```bash
./build/grayscale images/rose.jpg out.png
./build/grayscale images/rose.jpg out.png box
./build/grayscale images/rose.jpg out.png identity
./build/grayscale images/rose.jpg out.png sobelx
```

По умолчанию используется box.

## Запуск тестов

```bash
chmod +x run_all_tests.sh
./run_all_tests.sh
```

Скрипт автоматически прогоняет все изображения из папки images и сравнивает результат с OpenCV.

# Пример добавления своего изображения

```bash
cp ~/Downloads/your_image.jpg images/
```
