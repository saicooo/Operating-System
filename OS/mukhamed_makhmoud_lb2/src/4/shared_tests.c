/*
 * ======================================================================
 * Лабораторная работа 2, задание 4 (общие ресурсы потоков)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Проанализируйте ресурсы, разделяемые нитями одного процесса,
 * подтвердите экспериментально.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Создает группы потоков (нитей) используя функцию pthread_create().
 * Проводит серию тестов для демонстрации общих ресурсов потоков одного процесса:
 * - Доступ к текущей директории
 * - Доступ к переменным окружения
 * - Доступ к файловым дескрипторам
 * - Доступ к общему адресному пространству
 * - Обработка сигналов
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc shared_tests.c -o shared_tests -pthread
 * $ ./shared_tests
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Потоки одного процесса имеют общую текущую директорию (cwd)
 * - Потоки имеют доступ к одним и тем же переменным окружения
 * - Файловые дескрипторы разделяются между потоками процесса
 * - Все потоки работают в общем адресном пространстве (одинаковые адреса переменных)
 * - Потоки имеют общие обработчики сигналов
 * - Каждый поток имеет собственный идентификатор потока (thread ID),
 *   но все они имеют один и тот же идентификатор процесса (PID)
 */

#define _GNU_SOURCE

#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define _THREADS_COUNT 5
#define _FILENAME "shared_tests.log"

// Структура для передачи аргументов в функцию потока
typedef struct {
    size_t number; // Номер потока
    int fd;        // Номер файлового дескриптора
    int *var_ptr;  // Указатель на общую переменную
} pthread_args_t;

// Функция для проверки общедоступности текущей директории
void *test_cwd(void *arg) {
    // Получаем переданные аргументы
    pthread_args_t *info = (pthread_args_t *)arg;
    char cwd[1024]; // Задаем строку для хранения пути

    // Получаем путь к текущей директории
    getcwd(cwd, sizeof(cwd));

    printf("Поток [%lu]: Kernel ID (LWP)=[%d], PID=[%d], cwd=[%s]\n", info->number, gettid(),
           getpid(), cwd);
    return NULL;
}

// Функция для проверки общедоступности переменных окружежения
void *test_env(void *arg) {
    // Получаем переданные аргументы
    pthread_args_t *info = (pthread_args_t *)arg;

    // Получаем переменные окружения
    char *user = getenv("USER");
    char *home = getenv("HOME");

    printf("Поток [%lu]: Kernel ID (LWP)=[%d], PID=[%d], $USER=[%s], $HOME=[%s]\n", info->number,
           gettid(), getpid(), user, home);
    return NULL;
}

// Функция для проверки общедоступности файловых дескрипторов
void *test_fd(void *arg) {
    // Получаем переданные аргументы
    pthread_args_t *info = (pthread_args_t *)arg;

    // Создаем переменную-буфер и записываем в нее строку
    char buffer[128];
    sprintf(buffer, "Поток [%lu]: Kernel ID (LWP)=[%d], PID=[%d] записал информацию\n",
            info->number, getpid(), gettid());

    // Записываем строку в файл
    write(info->fd, buffer, strlen(buffer));
    return NULL;
}

// Функция для проверки общедоступности адресного пространства
void *test_memory(void *arg) {
    // Получаем переданные аргументы
    pthread_args_t *info = (pthread_args_t *)arg;

    // Увеличиваем общую переменную
    *(info->var_ptr++);

    printf("Поток [%lu]: Kernel ID (LWP)=[%d], PID=[%d], var_ptr=[%d], &var_ptr=[%p]\n",
           info->number, gettid(), getpid(), *info->var_ptr, info->var_ptr);

    return NULL;
}

// Функция-обработчик сигнала - вызывается при получении сигнала SIGUSR1
void handler(int sig) {
    printf("Обработчик: Kernel ID (LWP)=[%d], PID=[%d], signal=[%d]\n", gettid(), getpid(), sig);
}

// Функция для проверки общедоступности сигналов
void *test_sig(void *arg) {
    // Устанавливаем обработчик и посылаем сигнал
    signal(SIGUSR1, handler);
    raise(SIGUSR1);

    return NULL;
}

void run_test(void *(*test_function)(void *), pthread_t threads[], pthread_args_t args[]) {
    // Создаем поток с указателем на функцию для теста
    for (size_t i = 0; i < _THREADS_COUNT; i++) {
        if (pthread_create(&threads[i], NULL, test_function, &args[i]) != 0) {
            fprintf(stderr, "Ошибка pthread_create: невозможно создать поток");
            exit(EXIT_FAILURE);
        }
    }

    // Ожидаем завершения потоков
    for (size_t i = 0; i < _THREADS_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    // Отключаем буферизацию
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    printf("\n==== Демонстрация общих ресурсов потоков одного процесса ====\n");

    // Создаем отдельные массивы аргументов и идентификаторов потоков
    pthread_args_t threads_args[_THREADS_COUNT];
    pthread_t threads[_THREADS_COUNT];

    // Общая переменная
    int value = 0;

    // Открываем файл
    int fd = open(_FILENAME, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    for (size_t i = 0; i < _THREADS_COUNT; i++) {
        threads_args[i].number = i;
        threads_args[i].var_ptr = &value;
        threads_args[i].fd = fd;
    }

    // Запускаем тесты
    printf("\n--- Тест общедоступности текущей директории ---\n");
    run_test(test_cwd, threads, threads_args);

    printf("\n--- Тест общедоступности переменных окружежения ---\n");
    run_test(test_env, threads, threads_args);

    printf("\n--- Тест общедоступности файловых дескрипторов ---\n");
    run_test(test_fd, threads, threads_args);

    // Выводим содержимое файла
    char cmd[64];
    sprintf(cmd, "cat %s", _FILENAME);
    system(cmd);

    printf("\n--- Тест общедоступности адресного пространства ---\n");
    run_test(test_memory, threads, threads_args);

    printf("\n--- Тест общедоступности сигналов ---\n");
    run_test(test_sig, threads, threads_args);

    // Закрываем файл
    close(fd);

    return 0;
}