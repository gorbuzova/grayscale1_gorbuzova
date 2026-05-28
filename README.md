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
