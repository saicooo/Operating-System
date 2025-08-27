/*
 * ======================================================================
 * Лабораторная работа 2, задание 5.3 (планирование FIFO)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Исследование процедуры планирования FIFO в Linux с одинаковыми и разными
 * приоритетами процессов для анализа очередности предоставления процессора.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа устанавливает для себя политику планирования FIFO с заданным
 * приоритетом (получаемым из командной строки). Затем выполняет длительную
 * вычислительную работу в цикле, периодически выводя информацию о своем
 * состоянии: PID, PPID, политику и приоритет.
 * Запускается параллельно с prog1 для сравнения порядка выполнения
 * процессов при различных настройках приоритетов.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc prog2.c -o prog2
 * $ ./prog2 <приоритет>
 * (где <приоритет> - число из диапазона для FIFO, обычно 1-99)
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Совместно с prog1 демонстрирует работу планировщика FIFO
 * - При запуске с одинаковыми приоритетами показывает, что процессы
 *   выполняются последовательно по принципу FIFO (первый пришел - первый ушел)
 * - При запуске с разными приоритетами демонстрирует, что процесс с более
 *   высоким приоритетом получает процессорное время первым независимо
 *   от времени запуска (либо получает проц. время сразу после того, как планировщик
 *   установил политику FIFO: т.е. между запуском и установкой политики м.б. задержка)
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Функция для получения строкового представления политики планирования
char *get_policy_name(int policy) {
    char *policy_name;

    switch (policy) {
        case SCHED_FIFO:
            policy_name = "SCHED_FIFO";
            break;
        case SCHED_RR:
            policy_name = "SCHED_RR";
            break;
        case SCHED_OTHER:
            policy_name = "SCHED_OTHER";
            break;
        default:
            policy_name = "UNKNOWN";
            break;
    }

    return policy_name;
}

int main(int argc, char *argv[]) {
    // Получаем границы для возможного приоритета
    int min_priority = sched_get_priority_min(SCHED_FIFO);
    int max_priority = sched_get_priority_max(SCHED_FIFO);

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

    // Устанавливаем политику FIFO с заданным приоритетом
    struct sched_param param;
    param.sched_priority = priority;

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        fprintf(stderr, "Ошибка sched_setscheduler: невозможно изменить политику планирования\n");
        exit(EXIT_FAILURE);
    }

    // Имитация работы
    for (int i = 0; i < 10; i++) {
        // Создаем цикл for, что не будет оптимизирован компилятором
        volatile int counter = 0;
        for (size_t j = 0; j < 1000000000; j++) {
            counter++;
        }

        printf("prog2: PPID=[%d], PID=[%d], policy=[%s], priority=[%d]\n", getppid(), getpid(),
               get_policy_name(sched_getscheduler(0)), param.sched_priority);
    }

    return 0;
}