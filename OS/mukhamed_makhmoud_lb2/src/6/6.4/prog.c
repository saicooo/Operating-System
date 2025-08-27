/*
 * ======================================================================
 * Лабораторная работа 2, задание 6.4 (наследование атрибутов через clone())
 * ======================================================================
 *
 * ЗАДАЧА:
 * Исследование особенностей наследования различных атрибутов процесса при
 * использовании системного вызова clone(), который является более гибкой
 * альтернативой fork(). Определение, как наследуются файловые дескрипторы,
 * политики планирования и обработчики сигналов.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа выполняет три теста:
 * 1. Тест 6.4.1: Наследование файловых дескрипторов через clone().
 *    Открывает файл, создает потомка через clone() и проверяет, может ли
 *    потомок записывать в тот же файловый дескриптор.
 *
 * 2. Тест 6.4.2: Наследование политики планирования через clone().
 *    Устанавливает политику планирования SCHED_FIFO с приоритетом 50,
 *    создает потомка через clone() и проверяет, наследуются ли эти настройки.
 *
 * 3. Тест 6.4.3: Наследование сигналов через clone().
 *    Устанавливает различные обработчики и диспозиции сигналов, создает
 *    потомка через clone() и проверяет наследование через проверку обработчиков
 *    и отправку сигналов.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc prog.c -o prog
 * $ ./prog
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Файловые дескрипторы полностью наследуются через clone().
 * - Политика планирования и приоритет также наследуются через clone().
 * - Обработчики сигналов и их диспозиции наследуются, включая как пользовательские
 *   обработчики, так и игнорирование сигналов.
 * - Системный вызов clone() обеспечивает такое же наследование атрибутов,
 *   как и fork(), но с возможностью более тонкой настройки через флаги.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)  // Размер стека для нового процесса (1МБ)

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

// Функция, запускаемая в потомке clone для теста наследования файловых дескрипторов
int clone_child_fd(void *arg) {
    int fd = *((int *)arg);  // Получаем файловый дескриптор из аргумента
    const char *msg = "Потомок clone записал\n";
    if (write(fd, msg, strlen(msg)) < 0) {  // Пишем в файл
        fprintf(stderr, "Ошибка write: запись clone-потомка (fd)\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

// Функция, запускаемая в потомке clone для теста наследования политики планирования
int clone_child_sched(void *arg) {
    struct sched_param param;
    int policy = sched_getscheduler(0);  // Получаем политику планирования
    sched_getparam(0, &param);  // Получаем параметры планирования

    printf("Потомок clone: PPID=[%d], PID=[%d], policy=[%s], priority=[%d]\n", getppid(), getpid(),
           get_policy_name(policy), param.sched_priority);
    return 0;
}

// Функция для проверки диспозиции сигнала
void check_signal(int sig, const char *name) {
    struct sigaction sa;
    if (sigaction(sig, NULL, &sa) == -1) {  // Получаем текущую обработку сигнала
        perror("sigaction failed");
        return;
    }

    // Определяем, какой обработчик установлен
    if (sa.sa_handler == SIG_DFL) {
        printf("Сигнал %s (%d): обработчик по умолчанию\n", name, sig);
    } else if (sa.sa_handler == SIG_IGN) {
        printf("Сигнал %s (%d): игнорируется\n", name, sig);
    } else {
        printf("Сигнал %s (%d): пользовательский обработчик (%p)\n", name, sig, sa.sa_handler);
    }
}

// Функция, запускаемая в потомке clone для теста наследования сигналов
int clone_child_signal(void *arg) {
    printf("Потомок: PPID=[%d], PID=[%d]\n", getppid(), getpid());

    // Проверяем, какие обработчики сигналов унаследованы
    check_signal(SIGUSR1, "SIGUSR1");
    check_signal(SIGTERM, "SIGTERM");
    check_signal(SIGINT, "SIGINT");
    check_signal(SIGHUP, "SIGHUP");

    printf("Ожидаю сигнал...\n");
    pause();  // Ожидаем сигнал
    return 0;
}

// Обработчик сигнала для теста
void signal_handler(int sig) {
    printf("Обработчик сигнала %d в PID=[%d]\n", sig, getpid());
}

// Функция для тестирования наследования файловых дескрипторов через clone
void test_clone_fd() {
    printf("\n===== Тест 6.4.1: Наследование файловых дескрипторов через clone() =====\n");

    // Открываем файл для записи
    int fd = open("6.4_clone_fd.log", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        fprintf(stderr, "Ошибка open\n");
        exit(EXIT_FAILURE);
    }

    // Выделяем стек для нового процесса
    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        fprintf(stderr, "Ошибка malloc\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Запись родителя до clone
    const char *parent_msg = "Родитель записал до clone\n";
    if (write(fd, parent_msg, strlen(parent_msg)) < 0) {
        fprintf(stderr, "Ошибка write (родитель до clone)\n");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Создаем новый процесс через clone с передачей файлового дескриптора
    pid_t pid = clone(clone_child_fd, stack + STACK_SIZE, SIGCHLD | CLONE_FILES, &fd);
    if (pid < 0) {
        fprintf(stderr, "Ошибка clone\n");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);  // Ждем завершения потомка

    // Запись родителя после clone
    const char *parent_msg2 = "Родитель записал после clone\n";
    if (write(fd, parent_msg2, strlen(parent_msg2)) < 0) {
        fprintf(stderr, "Ошибка write (родитель после clone)\n");
        free(stack);
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);  // Закрываем файл
    free(stack);  // Освобождаем память стека
    printf("Файл 6.4_clone_fd.log создан\n");
    system("echo '\n--- Cодержимое файла 6.4_clone_fd.log ---\n' && cat 6.4_clone_fd.log");
}

// Функция для тестирования наследования политики планирования через clone
void test_clone_sched() {
    printf("\n===== Тест 6.4.2: Наследование политики планирования через clone() =====\n");

    // Устанавливаем политику и приоритет для текущего процесса
    struct sched_param param;
    param.sched_priority = 50;
    if (sched_setscheduler(0, SCHED_FIFO, &param) < 0) {
        fprintf(stderr, "Ошибка sched_setscheduler\n");
        exit(EXIT_FAILURE);
    }

    // Выделяем стек для нового процесса
    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        fprintf(stderr, "Ошибка malloc\n");
        exit(EXIT_FAILURE);
    }

    printf("Родитель: PPID=[%d], PID=[%d], policy=[%s], priority=[%d]\n", getppid(), getpid(),
           get_policy_name(SCHED_FIFO), param.sched_priority);

    // Создаем новый процесс через clone
    pid_t pid = clone(clone_child_sched, stack + STACK_SIZE, SIGCHLD, NULL);
    if (pid < 0) {
        fprintf(stderr, "Ошибка clone\n");
        free(stack);
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);  // Ждем завершения потомка
    free(stack);  // Освобождаем память стека
}

// Функция для тестирования наследования обработки сигналов через clone
void test_clone_signals() {
    printf("\n===== Тест 6.4.3: Наследование сигналов через clone() =====\n");

    // Настраиваем пользовательский обработчик сигналов
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // Устанавливаем обработчики для нескольких сигналов
    if (sigaction(SIGUSR1, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, "Ошибка sigaction: невозможно установить обработчики сигналов\n");
        exit(EXIT_FAILURE);
    }

    // Игнорируем SIGHUP в родителе
    signal(SIGHUP, SIG_IGN);

    // Выделяем стек для нового процесса
    char *stack = malloc(STACK_SIZE);
    if (!stack) {
        fprintf(stderr, "Ошибка malloc: невозможно выделить память\n");
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;

    // Добавляем CLONE_SIGHAND для наследования обработчиков сигналов
    int clone_flags = CLONE_VM | CLONE_SIGHAND | SIGCHLD;

    // Создаем новый процесс через clone
    pid_t child_pid = clone(clone_child_signal, stack_top, clone_flags, NULL);
    if (child_pid == -1) {
        fprintf(stderr, "Ошибка clone: невозможно создать потомка\n");
        free(stack);
        exit(EXIT_FAILURE);
    }

    sleep(1);  // Даем потомку время на запуск
    printf("\nРодитель отправляет сигналы:\n");
    kill(child_pid, SIGUSR1);  // Отправляем сигнал с пользовательским обработчиком
    kill(child_pid, SIGHUP);   // Отправляем игнорируемый сигнал

    waitpid(child_pid, NULL, 0);  // Ждем завершения потомка
    free(stack);  // Освобождаем память стека
}

// Главная функция программы
int main() {
    test_clone_fd();     // Тест наследования файловых дескрипторов
    test_clone_sched();  // Тест наследования политики планирования
    test_clone_signals();  // Тест наследования обработки сигналов
    return 0;
}