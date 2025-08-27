/*
 * ======================================================================
 * Лабораторная работа 2, задание 5.4 (планирование RR с sched_yield)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Исследование влияния функции sched_yield() на порядок очереди в процедуре
 * планирования Round Robin (RR) в Linux.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа устанавливает для себя политику планирования RR с заданным
 * приоритетом (получаемым из командной строки). Затем выполняет цикл с
 * вычислительной работой, но после каждой итерации вызывает функцию
 * sched_yield(), которая добровольно уступает процессорное время другим
 * процессам с тем же приоритетом, перемещая текущий процесс в конец
 * очереди процессов.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc yield.c -o yield
 * $ ./yield <приоритет>
 * (где <приоритет> - число из диапазона для RR, обычно 1-99)
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Демонстрирует влияние sched_yield() на поведение планировщика RR
 * - Позволяет наблюдать, как процесс добровольно отказывается от оставшегося
 *   времени своего кванта и перемещается в конец очереди процессов
 * - При запуске с другими процессами показывает, как происходит изменение
 *   порядка выполнения процессов в очереди RR благодаря yield
 * - Иллюстрирует механизм кооперативной многозадачности внутри
 *   политики вытесняющей многозадачности RR
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

    // Имитация работы
    for (int i = 0; i < 10; i++) {
        printf("yield: PPID=[%d], PID=[%d], policy=[%s], priority=[%d]\n", getppid(), getpid(),
               get_policy_name(sched_getscheduler(0)), param.sched_priority);
        sched_yield();

        // Создаем цикл for, что не будет оптимизирован компилятором
        volatile int counter = 0;
        for (size_t j = 0; j < 100000000; j++) {
            counter++;
        }
    }

    return 0;
}
