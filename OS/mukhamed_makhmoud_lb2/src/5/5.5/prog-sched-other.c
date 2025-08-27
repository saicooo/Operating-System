/*
 * ======================================================================
 * Лабораторная работа 2, задание 5.5 (конкуренция разных политик планирования)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Исследование возможности задания разных процедур планирования разным 
 * процессам с одинаковыми приоритетами и анализ их конкуренции за процессорное 
 * время. Изучение понятия приоритетности по отношению к политике планирования.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа устанавливает для себя стандартную политику планирования OTHER 
 * с максимально возможным nice-приоритетом (-20). Затем выполняет длительную 
 * вычислительную работу в цикле, периодически выводя информацию о своем 
 * состоянии: PID, PPID, nice-приоритет и политику планирования.
 * SCHED_OTHER - стандартная политика планирования Linux для обычных процессов, 
 * отличающаяся от политик реального времени (FIFO, RR) более низким приоритетом 
 * выполнения независимо от числового значения приоритета процесса.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc prog-sched-other.c -o prog-sched-other
 * $ ./prog-sched-other
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Демонстрирует работу процесса с обычной политикой планирования при 
 *   конкуренции с процессами реального времени
 * - Показывает, что процесс с политикой SCHED_OTHER выполняется только когда 
 *   нет готовых к выполнению процессов реального времени
 * - Политика планирования имеет больший приоритет, чем числовое значение 
 *   приоритета процесса (даже с максимальным nice=-20 процесс SCHED_OTHER 
 *   уступает процессам реального времени)
 */

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

// Функция для получения типа планирования в строковом виде
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
    // Устанавливаем политику OTHER с заданным приоритетом
    struct sched_param param = {0};
    if (sched_setscheduler(0, SCHED_OTHER, &param) == -1) {
        fprintf(stderr, "Ошибка sched_setscheduler: невозможно изменить политику планирования\n");
        exit(EXIT_FAILURE);
    }

    if (setpriority(PRIO_PROCESS, 0, -20) == -1) {
        fprintf(stderr, "Ошибка setpriority: не удается изменить значение\n");
    }

    // Получаем приоритет процесса
    int priority = getpriority(PRIO_PROCESS, 0);
    if (errno != 0) {
        fprintf(stderr, "Ошибка getpriority: не удается получить значение\n");
        exit(EXIT_FAILURE);
    }

    // Имитация работы
    for (int i = 0; i < 10; i++) {
        // Создаем цикл for, что не будет оптимизирован компилятором
        volatile int counter = 0;
        for (size_t j = 0; j < 1000000000; j++) {
            counter++;
        }

        printf("prog-sched-other: PPID=[%d], PID=[%d], policy=[%s], nice=[%d]\n", getppid(),
               getpid(), get_policy_name(sched_getscheduler(0)), priority);
    }

    return 0;
}
