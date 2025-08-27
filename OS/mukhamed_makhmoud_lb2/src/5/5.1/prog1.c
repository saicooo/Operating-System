/*
 * ======================================================================
 * Лабораторная работа 2, задание 5.1 (планирование процессов)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Определение политики планирования и приоритета, установленных по
 * умолчанию, для процессов, запускаемых пользователем из shell. Анализ
 * очередности исполнения процессов при различных приоритетах и политиках.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа prog1 является тестовой программой, которая выводит
 * информацию о себе. Используется для анализа очередности
 * исполнения процессов при их одновременном запуске с разными приоритетами
 * и политиками планирования. Выводит PID, PPID и текущую политику планирования.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc prog1.c -o prog1
 * $ ./prog1
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Подтверждает, что процессы с одинаковым приоритетом получают примерно
 *   равное количество процессорного времени (чередование вывода).
 * - При запуске с высоким nice значением (10) получает наименьшее количество
 *   процессорного времени среди всех запущенных процессов.
 * - Демонстрирует, что планировщик Linux распределяет процессорное время
 *   в зависимости от nice значений, даже если все процессы запущены с одним ядром.
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
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

    if (argc < 2) {
        fprintf(stderr, "Ошибка argc: необходимо задать приоритет\n");
        exit(EXIT_FAILURE);
    }

    int priority = atoi(argv[1]);

    if (setpriority(PRIO_PROCESS, 0, priority) == -1) {
        fprintf(stderr, "Ошибка setpriority: не удается изменить значение\n");
        exit(EXIT_FAILURE);
    }

    // Имитация работы
    for (int i = 0; i < 10; i++) {
        // Создаем цикл for, что не будет оптимизирован компилятором
        volatile int counter = 0;
        for (size_t j = 0; j < 1000000000; j++) {
            counter++;
        }

        printf("prog1: PPID=[%d], PID=[%d], policy=[%s], priority=[%d]\n", getppid(), getpid(),
               get_policy_name(sched_getscheduler(0)), getpriority(PRIO_PROCESS, 0));
    }

    return 0;
}
