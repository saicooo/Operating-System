/*
 * ======================================================================
 * Лабораторная работа 2, задание 4 (многонитевое функционирование)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Демонстрация многонитевого функционирования в Linux, создание нитей
 * разными способами, анализ их идентификации, распределения во времени
 * и разделяемых ресурсов. Изучение влияния удаления нити через kill.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Создает две группы потоков (нитей) используя функции pthread_create()
 * и clone(). Выводит информацию о созданных потоках, включая их
 * идентификаторы, адреса разделяемых данных. Демонстрирует, что потоки
 * одного процесса имеют доступ к общей памяти. В конце выполняет
 * принудительное удаление одного из потоков через kill для анализа
 * последствий.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc threads.c -o threads
 * $ ./threads
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Показывает, что нити (потоки) имеют различные идентификаторы, но
 *   работают в общем адресном пространстве процесса.
 * - Демонстрирует разницу между POSIX threads API и системным вызовом clone().
 * - Иллюстрирует, что при удалении отдельной нити через kill весь процесс
 *   может быть завершен, так как нити разделяют общие ресурсы.
 * - Подтверждает, что нити одного процесса разделяют переменные, что
 *   видно по одинаковым адресам переменных в разных потоках.
 */

#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define _THREADS_COUNT 5
#define _STACK (1024 * 1024)

// Структура для передачи аргументов в функцию потока
typedef struct {
    size_t number;
    int   *value_ptr;
} pthread_args_t;

void *create_info(void *arg) {
    pthread_args_t *pthread_args = (pthread_args_t *)arg;

    printf("Поток [%lu]: Kernel ID (LWP)=[%d], val=[%d], &val=[%p]\n", pthread_args->number,
           gettid(), *pthread_args->value_ptr, pthread_args->value_ptr);

    sleep(1); // Используем задержку для корректного отображения информации о потоке через ps -Ll

    return NULL;
}

int clone_info(void *arg) {
    pthread_args_t *pthread_args = (pthread_args_t *)arg;

    printf("Поток [%lu]: Kernel ID (LWP)=[%d], val=[%d], &val=[%p]\n", pthread_args->number,
           gettid(), *pthread_args->value_ptr, pthread_args->value_ptr);

    sleep(1); // Используем задержку для корректного отображения информации о потоке через ps -Ll

    return 0;
}

int main() {
    // Отключаем буферизацию
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    char cmd[256];    // инициализируем переменную для вывода дополнительной информации на экран
    int  value = 100; // Создаем отдельную переменную для проверки ресурсов, разделяемые нитями в
                      // одном процессе

    pthread_args_t threads_args[_THREADS_COUNT]; // Создаем отдельный массив идентификаторов потоков

    printf("\n==== Демонстрация работы потоков ====\n");

    // Создание потоков через pthread_create()
    printf("\n--- Создание потоков через pthread_create() ---\n");
    pthread_t threads[_THREADS_COUNT]; // Создание массива потоков

    for (size_t i = 0; i < _THREADS_COUNT; i++) {
        threads_args[i].number    = i;
        threads_args[i].value_ptr = &value;

        //  Создаем отдельный поток и передаем указатель на функцию create_info, что будет
        //  применяться в потоке
        if (pthread_create(&threads[i], NULL, create_info, (void *)&threads_args[i]) != 0) {
            fprintf(stderr, "Ошибка создания потока [%lu]", i);
            exit(EXIT_FAILURE);
        }
    }

    sprintf(cmd,
            "echo '\n--- Информация о потоках, созданных через pthread_create() ---' && ps -Ll");
    system(cmd);

    // Ожидаем завершения всех потоков, созданных через pthread_create()
    for (size_t i = 0; i < _THREADS_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    // Создание потоков через clone()
    printf("\n--- Создание потоков через clone() ---\n");

    // Массив для хранения PID потоков, созданных через clone
    pid_t clone_pids[_THREADS_COUNT];

    // Создаем стеки для потоков clone
    void *treads_stack[_THREADS_COUNT];

    // Флаги для clone - создаем поток, который разделяет с родителем
    // адресное пространство, файловые дескрипторы и обработчики сигналов
    int clone_flags =
        CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM;

    for (size_t i = 0; i < _THREADS_COUNT; i++) {
        // Выделяем память для стека
        treads_stack[i] = malloc(_STACK);
        if (!treads_stack[i]) {
            fprintf(stderr, "Ошибка выделения памяти для стека потока [%lu]\n", i);
            exit(EXIT_FAILURE);
        }

        // Обновляем значения в структуре данных
        threads_args[i].number    = i;
        threads_args[i].value_ptr = &value;

        // Создаем поток с помощью clone()
        // Стек растет вниз, поэтому указываем на конец выделенной области
        clone_pids[i] =
            clone(clone_info, (char *)treads_stack[i] + _STACK, clone_flags, &threads_args[i]);

        if (clone_pids[i] == -1) {
            fprintf(stderr, "Ошибка создания потока [%lu]\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Выводим информацию о созданных потоках через clone
    sprintf(cmd, "echo '\n--- Информация о потоках, созданных через clone() ---' && ps -Ll");
    system(cmd);

    // Ожидаем завершения потоков
    for (size_t i = 0; i < _THREADS_COUNT; i++) {
        waitpid(clone_pids[i], NULL, 0);
    }

    // Освобождаем память, выделенную для стеков
    for (size_t i = 0; i < _THREADS_COUNT; i++) {
        free(treads_stack[i]);
    }

    // Удаление потока
    pthread_t      thread_to_kill;
    pthread_args_t args = {.number = 6, .value_ptr = &value};

    int status = pthread_create(&thread_to_kill, NULL, create_info, (void *)&args);

    printf("\n--- Удаление последнего потока (LWP=%d) ---\n", gettid());
    kill(gettid(), SIGKILL); // Посылаем сигнал SIGKILL потоку (приводит к
                             // завершению всей программы)

    printf("\n==== Демонстрация завершена ====\n");

    return 0;
}