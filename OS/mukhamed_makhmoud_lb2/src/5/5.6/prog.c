/*
 * ======================================================================
 * Лабораторная работа 2, задание 5.6 (планирование потоков одного процесса)
 * ======================================================================
 *
 * ЗАДАЧА:
 * 1. Определение политики планирования по умолчанию для потоков одного процесса
 * 2. Исследование влияния изменения политики планирования на потоки одного процесса
 * 3. Анализ влияния разных приоритетов на очередность исполнения потоков одного процесса
 * 4. Сравнение поведения потоков одного процесса с поведением независимых процессов
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа выполняет несколько экспериментов:
 * 1. Создает поток с настройками по умолчанию для определения политики планирования по умолчанию
 * 2. Запускает несколько потоков с одинаковой политикой SCHED_FIFO и одинаковым приоритетом
 * 3. Запускает три потока с разными политиками планирования (SCHED_OTHER, SCHED_RR, SCHED_FIFO)
 * 4. Запускает два потока одного процесса с политикой SCHED_FIFO и разными приоритетами (10 и 99)
 * 5. Запускает два потока независимых процессов с политикой SCHED_FIFO и разными приоритетами
 * В каждом потоке выполняется интенсивная вычислительная работа, и выводится информация
 * о TID, политике планирования и приоритете.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc prog.c -o prog -lpthread
 * $ taskset -c 0 ./prog   # Запуск с привязкой к одному ядру процессора
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Политика планирования по умолчанию для потоков - SCHED_OTHER
 * - Политики реального времени (SCHED_FIFO, SCHED_RR) имеют преимущество над SCHED_OTHER
 * - При SCHED_FIFO для потоков одного процесса с разными приоритетами, поток с высоким
 *   приоритетом (99) выполняется перед потоком с низким приоритетом (10)
 * - Для потоков независимых процессов с SCHED_FIFO порядок выполнения не зависит от приоритета
 * - Установка разных приоритетов при политике SCHED_FIFO влияет на очередность исполнения
 *   как для потоков одного процессаы
 */

#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define _THREADS_COUNT 3
#define _PRIORITY 10
#define _POLICY SCHED_FIFO

// Структура для задачи параметров для потока
typedef struct {
    int       policy;
    int       priority;
    pthread_t thread;
    char      name[32];
} pthread_config_t;

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

// Функция для получения информации о политике планирования потока
void get_info(char *name) {
    // Вводим переменные, куда будет записана информация о приоритете и политике планирования для
    // потока
    pthread_t current = pthread_self();

    int                policy;
    struct sched_param param;

    // Получаем параметры политики планирования для текущего потока
    if (pthread_getschedparam(current, &policy, &param) != 0) {
        fprintf(stderr, "Ошибка pthread_getschedparam: невозможно получить информацию о потоке\n");
        exit(EXIT_FAILURE);
    }

    // Выводим результат
    printf("Поток [%s]: TID=[%d], policy=[%s], priority=[%d]\n", name, gettid(),
           get_policy_name(policy), param.sched_priority);
}

// Функция для имитации работы потока
void *thread_function(void *arg) {
    char *name = (char *)arg;

    for (int i = 0; i < 5; i++) {
        // Создаем цикл for, что не будет оптимизирован компилятором
        volatile int counter = 0;
        for (size_t j = 0; j < 1000000000; j++) {
            counter++;
        }
        get_info(name);
    }
}

// Функция для создания потоков с измененными начальными данными
int create_thread(pthread_t *thread, int policy, int priority, char *name) {
    pthread_attr_t attr;
    int            ret = pthread_attr_init(&attr);
    if (ret != 0) {
        fprintf(stderr, "Ошибка pthread_attr_init: не удалось инициализировать атрибуты потока\n");
        return ret;
    }

    // Устанавливаем политику планирования
    ret = pthread_attr_setschedpolicy(&attr, policy);
    if (ret != 0) {
        fprintf(stderr, "Ошибка pthread_attr_setschedpolicy: невозможно установить политику "
                        "планирования для потока\n");
        pthread_attr_destroy(&attr);
        return ret;
    }

    // Устанавливаем параметры политики
    struct sched_param param;
    param.sched_priority = priority;

    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret != 0) {
        fprintf(stderr, "Ошибка pthread_attr_setschedparam: невозможно установить параметры "
                        "планирования для потока\n");
        pthread_attr_destroy(&attr);
        return ret;
    }

    // Устанавливаем наследование тех параметров, что явно указаны в атрибутах
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret != 0) {
        fprintf(stderr,
                "Ошибка pthread_attr_setinheritsched: невозможно изменить вид наследования\n");
        pthread_attr_destroy(&attr);
        return ret;
    }

    // Запускаем поток
    ret = pthread_create(thread, &attr, thread_function, (void *)name);

    if (ret != 0) {
        fprintf(stderr, "Ошибка pthread_create: не удалось создать поток\n");
        pthread_attr_destroy(&attr);
        return ret;
    }

    // Высвобождаем ресурсы
    ret = pthread_attr_destroy(&attr);
    if (ret != 0) {
        fprintf(stderr, "Ошибка pthread_attr_destroy: не удалось освободить ресурсы атрибутов\n");
        return ret;
    }

    return EXIT_SUCCESS;
}

// Функция для запуска потоков
void run_threads(pthread_config_t threads[], size_t count) {
    // Запускаем потоки с нужными параметрами
    for (size_t i = 0; i < count; i++) {
        create_thread(&threads[i].thread, threads[i].policy, threads[i].priority, threads[i].name);
    }

    // Ожидаем завершение потоков
    for (size_t i = 0; i < count; i++) {
        pthread_join(threads[i].thread, NULL);
    }
}

int main() {
    // Создаем строку для вывода в терминал
    char cmd[256];

    // Определяем политику планирования по умолчанию
    sprintf(cmd, "echo '\n===== Определяем политику планирования по умолчанию =====\n'");
    system(cmd);

    pthread_t default_thread;
    pthread_create(&default_thread, NULL, thread_function, (void *)"0");
    pthread_join(default_thread, NULL); // Ожидаем завершения потока

    {
        // Создаем массив структур для описания потоков
        pthread_config_t threads[_THREADS_COUNT];

        // Устанавливаем для всех потоков равного приоритета одну политику
        sprintf(cmd, "echo '\n===== Изменяем политику планирования для всех потоков =====\n'");
        system(cmd);
        for (size_t i = 0; i < _THREADS_COUNT; i++) {
            threads[i].policy   = _POLICY;
            threads[i].priority = _PRIORITY;
            sprintf(threads[i].name, "%lu", i);
        }

        run_threads(threads, _THREADS_COUNT);
    }

    {
        pthread_config_t threads[3];

        sprintf(cmd, "echo '\n===== Запускаем 3 потока с разной политикой ====='");
        system(cmd);

        int policy[]   = {SCHED_OTHER, SCHED_RR, SCHED_FIFO};
        int priority[] = {0, _PRIORITY, _PRIORITY};

        for (size_t i = 0; i < 3; i++) {
            threads[i].policy   = policy[i];
            threads[i].priority = priority[i];
            sprintf(threads[i].name, "%lu", i);
        }

        sprintf(cmd, "echo '\n--- Порядок запуска ---\n1)%s\n2)%s\n3)%s\n'",
                get_policy_name(policy[0]), get_policy_name(policy[1]), get_policy_name(policy[2]));
        system(cmd);

        run_threads(threads, 3);
    }

    {
        pthread_config_t threads[2];

        sprintf(cmd,
                "echo '\n===== Запускаем 2 потока одного процесса с разным приоритетом ====='");
        system(cmd);

        int policy[]   = {SCHED_FIFO, SCHED_FIFO};
        int priority[] = {10, 99};

        for (size_t i = 0; i < 2; i++) {
            threads[i].policy   = policy[i];
            threads[i].priority = priority[i];
            sprintf(threads[i].name, "%lu-parent", i);
        }

        sprintf(cmd, "echo '\n--- Запуск с политикой %s ---\n'", get_policy_name(policy[0]));
        system(cmd);

        run_threads(threads, 2);

        sprintf(
            cmd,
            "echo '\n===== Запускаем 2 потока независимых процессов с разным приоритетом ====='");
        system(cmd);

        sprintf(cmd, "echo '\n--- Запуск с политикой %s ---\n'", get_policy_name(policy[0]));
        system(cmd);

        run_threads(&threads[0], 1);

        pid_t pid = fork();
        if (pid == 0) {
            // Код потомка
            sprintf(threads[1].name, "0-child");
            run_threads(&threads[1], 1);
        }

        if (pid > 0) {
            // Код родителя
            waitpid(pid, NULL, 0);
        }
    }

    return 0;
}