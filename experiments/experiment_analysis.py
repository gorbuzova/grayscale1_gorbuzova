import math
import sys

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np
from scipy import stats

def read_measurements(file_name):
    measurements = []
    file = open(file_name, "r")
    for line in file:
        text = line.strip()
        if text != "":
            measurements.append(float(text))
    file.close()
    return np.array(measurements)

def round_error(error):
    # Округляем погрешность до одной значащей цифры (или до двух, если первая
    # значащая цифра - единица)
    exponent = math.floor(math.log10(error))
    leading_digit = math.floor(error / (10 ** exponent))
    if leading_digit == 1:
        number_of_significant_figures = 2
    else:
        number_of_significant_figures = 1
    number_of_decimals = number_of_significant_figures - 1 - exponent
    rounded_error = round(error, number_of_decimals)
    return rounded_error, number_of_decimals

def format_value(value, number_of_decimals):
    # Если округляем до целых и крупнее - печатаем без дробной части
    if number_of_decimals <= 0:
        return str(int(round(value, number_of_decimals)))
    return str(round(value, number_of_decimals))

def analyze_measurements(measurements, name):
    print("Анализ серии измерений:", name)
    print("Количество измерений:", len(measurements))

    # Строим гистограмму
    plt.figure()
    plt.hist(measurements)
    plt.title("Гистограмма: " + name)
    plt.xlabel("Время, мс")
    plt.ylabel("Количество измерений")
    histogram_file = "histogram_" + name + ".png"
    plt.savefig(histogram_file)
    plt.close()
    print("Гистограмма сохранена в файл:", histogram_file)

    # Проверка на нормальность пройдена, если p-value > 0.05
    # хотя бы на одном тесте
    normaltest_result = stats.normaltest(measurements)
    shapiro_result = stats.shapiro(measurements)
    print("p-value (normaltest):", normaltest_result.pvalue)
    print("p-value (shapiro):", shapiro_result.pvalue)
    if normaltest_result.pvalue > 0.05 or shapiro_result.pvalue > 0.05:
        print("Хотя бы один тест на нормальность пройден")
    else:
        print("Тесты на нормальность не пройдены")

    # Среднее и стандартное отклонение
    mean_value = np.mean(measurements)
    standard_deviation = np.std(measurements, ddof=1)
    relative_deviation = standard_deviation / mean_value * 100
    print("Среднее отклонение:", mean_value, "мс")
    print("Стандартное отклонение:", standard_deviation, "мс")
    print("Отношение отклонения к среднему:", relative_deviation, "%")
    if relative_deviation > 10:
        print("Стандартное отклонение больше 10% от среднего")

    # Доверительный интервал, уровень доверия 95%
    confidence_interval = (stats.t.ppf(0.975, df=len(measurements) - 1)
                           * stats.sem(measurements))
    print("Случайная погрешность (95%):", confidence_interval, "мс")

    # Округление и итоговая запись результата
    rounded_error, number_of_decimals = round_error(confidence_interval)
    print("Результат:",
          format_value(mean_value, number_of_decimals),
          "+/-",
          format_value(rounded_error, number_of_decimals),
          "мс")

    return mean_value, confidence_interval

def main():
    if len(sys.argv) != 3:
        print("Использование: python experiment_analysis.py "
              "<файл_моих_измерений> <файл_измерений_opencv>")
        return

    my_measurements = read_measurements(sys.argv[1])
    opencv_measurements = read_measurements(sys.argv[2])

    my_mean, my_error = analyze_measurements(my_measurements,
                                             "my_implementation")
    opencv_mean, opencv_error = analyze_measurements(opencv_measurements,
                                                     "opencv")

    # Сравнение реализаций (так как времена сильно разного порядка, сравниваем
    # через относительные погрешности)
    print("Сравнение реализаций")
    ratio = my_mean / opencv_mean
    my_relative_error = my_error / my_mean
    opencv_relative_error = opencv_error / opencv_mean
    ratio_relative_error = math.sqrt(my_relative_error ** 2
                                     + opencv_relative_error ** 2)
    ratio_error = ratio * ratio_relative_error
    rounded_ratio_error, number_of_decimals = round_error(ratio_error)
    print("Отношение среднего времени (моя реализация / OpenCV):",
          format_value(ratio, number_of_decimals),
          "+/-",
          format_value(rounded_ratio_error, number_of_decimals))
    if ratio > 1:
        print("OpenCV быстрее в", format_value(ratio, number_of_decimals),
              "раз(а).")
    else:
        print("Моя реализация быстрее в",
              format_value(1 / ratio, number_of_decimals), "раз(а).")

if __name__ == "__main__":
    main()
