/*
 * ======================================================================
 * Лабораторная работа 2, задание 5.4 (определение кванта времени RR)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Определение величины кванта времени, используемого в политике планирования
 * Round Robin (RR) в Linux.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа устанавливает для себя политику планирования RR с заданным
 * приоритетом (получаемым из командной строки). Затем использует системный
 * вызов sched_rr_get_interval() для получения текущего значения кванта
 * времени для политики RR. Дополнительно получает значение кванта из 
 * системного файла /proc/sys/kernel/sched_rr_timeslice_ms для проверки.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc quant.c -o quant
 * $ ./quant <приоритет>
 * (где <приоритет> - число из диапазона для RR, обычно 1-99)
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Показывает величину кванта времени в политике RR в секундах и наносекундах
 * - Демонстрирует использование системного вызова sched_rr_get_interval()
 * - Дополнительно проверяет значение кванта через файл /proc/sys/kernel/sched_rr_timeslice_ms
 * - Полученное значение кванта позволяет лучше понять, как часто происходит
 *   переключение между процессами с одинаковым приоритетом в политике RR
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Получаем границы для возможного приоритета
    int min_priority = sched_get_priority_min(SCHED_RR);
    int max_priority = sched_get_priority_max(SCHED_RR);

    int priority; // Задаем переменную для получения приоритета программы из вне

    if (argc > 1) {
        priority = atoi(argv[1]); // Получаем приоритет из терминала

        if (priority < min_priority || priority > max_priority) {
            fprintf(stderr, "Ошибка priority: приоритет должен задаваться в диапазоне [%d; %d]\n",
                    min_priority, max_priority);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Ошибка приоритета: приоритет должен быть установлен\n");
        exit(EXIT_FAILURE);
    }

    // Устанавливаем политику RR с заданным приоритетом
    struct sched_param param;
    param.sched_priority = priority;
    
    if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
        fprintf(stderr, "Ошибка sched_setscheduler: невозможно изменить политику планирования\n");
        exit(EXIT_FAILURE);
    }

    // Структура для хранения значения кванта
    struct timespec quant;

    // Получаем значение кванта RR для процесса
    if (sched_rr_get_interval(0, &quant) == -1) {
        fprintf(stderr, "Ошибка sched_rr_get_interval: невозможно измерить квант RR");
        exit(EXIT_FAILURE);
    }

    // Выводим результат
    printf("quant: Величина кванта RR: %ld.%09ld секунд\n", quant.tv_sec, quant.tv_nsec);
    system("echo 'quant: Величина кванта, определенная в /proc/sys/kernel/sched_rr_timeslice_ms: ' "
           "&& cat /proc/sys/kernel/sched_rr_timeslice_ms");

    return 0;
}