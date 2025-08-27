/*
 * ======================================================================
 * Лабораторная работа 2, задание 6.2 (наследование приоритетов и планирования)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Проверка сохранения параметров планирования при выполнении системного вызова
 * exec(). Программа запускается из потомка, созданного в father.c.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа выводит информацию о своей политике планирования и приоритете
 * после замены образа процесса через exec(). Подтверждает, что настройки
 * SCHED_FIFO с приоритетом 50, установленные в родительском процессе,
 * сохраняются после exec().
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc son.c -o son
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Политика планирования и приоритет процесса сохраняются после
 *   выполнения exec(), что подтверждает их наследование через эту
 *   системную функцию.
 */

#include <errno.h>
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

int main() {
    // Получаем и выводим информацию о текущей политике планирования
    int policy = sched_getscheduler(0);
    if (policy < 0) {
        fprintf(stderr, "Ошибка sched_getscheduler: невозможно получить политику планирования\n");
        exit(EXIT_FAILURE);
    }

    // Получаем параметры планирования (включая приоритет)
    struct sched_param param;
    if (sched_getparam(0, &param) < 0) {
        fprintf(stderr, "Ошибка sched_getparam: невозможно получить параметры планирования\n");
        exit(EXIT_FAILURE);
    }

    // Выводим информацию о процессе и его параметрах планирования
    printf("Потомок (exec): PPID=[%d], PID=[%d], policy=[%s], priority=[%d]\n", getppid(), getpid(),
           get_policy_name(policy), param.sched_priority);

    return 0;
}