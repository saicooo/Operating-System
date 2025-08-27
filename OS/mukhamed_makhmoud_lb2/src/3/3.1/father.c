/*
 * ======================================================================
 * Лабораторная работа 2, задание 3.1 (родительский процесс)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Демонстрация различных реакций процессов на сигналы в Linux:
 * а) Реакция по умолчанию (son1)
 * б) Игнорирование сигнала (son2)
 * в) Перехват и обработка сигнала (son3)
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Создает три дочерних процесса (son1, son2, son3), каждый из которых
 * запускается через execl. После запуска потомков программа выводит
 * таблицу процессов, отправляет сигнал SIGUSR1 каждому потомку,
 * снова выводит таблицу процессов и завершает работу.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc father.c -o father
 * $ gcc son1.c -o son1
 * $ gcc son2.c -o son2
 * $ gcc son3.c -o son3
 * $ ./father
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - son1: Процесс завершается по умолчанию при получении сигнала SIGUSR1
 * - son2: Процесс игнорирует сигнал SIGUSR1 и продолжает выполнение
 * - son3: Процесс перехватывает сигнал SIGUSR1 и обрабатывает его,
 *   выводя сообщение о получении сигнала и продолжая выполнение
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Функция для создания и запуска потомков
pid_t run_child(const char *childname) {
    // Копируем родителя
    pid_t child = fork();
    if (child < 0) {
        fprintf(stderr, "Ошибка создания [%s]\n", childname);
        exit(EXIT_FAILURE);
    }

    // Запуск кода потомка
    if (child == 0) {
        execl(childname, childname, NULL); // Запуск без дополнительных параметров
        fprintf(stderr, "Ошибка execl для [%s]\n", childname);
        exit(EXIT_FAILURE);
    }

    return child;
}

int main(int argc, char *argv[]) {
    // Отключаем буферизацию
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    char cmd[256]; // Создаем переменную для вывода инфомрации в консоль

    sprintf(cmd, "\n--- Информация о father ---\nPID=[%d], PPID=[%d]\n\n", getpid(), getppid());
    printf("%s", cmd);

    // Создание и запуск потомков
    pid_t son1 = run_child("son1");
    pid_t son2 = run_child("son2");
    pid_t son3 = run_child("son3");

    sleep(1); // Даем потомкам время запуститься (для корректного запуска потомков)

    sprintf(
        cmd,
        "echo '\n--- Вывод ps от процесса-отца (PID=%d) до отправки сигналов ---' && ps -l",
        getpid());
    system(cmd);
    printf("\n");

    // Отправка сигналов
    kill(son1, SIGUSR1);
    kill(son2, SIGUSR1);
    kill(son3, SIGUSR1);

    sleep(1); // Ждем некоторое время

    sprintf(cmd, "echo '\n--- Вывод ps от процесса-отца (PID=%d) после отправки сигналов ---' "
                 "&& ps -l", getpid());
    system(cmd);
    printf("\n");

    // Принудительно завершаем процессы, которые не завершились
    kill(son1, SIGKILL);
    kill(son2, SIGKILL);
    kill(son3, SIGKILL);

    // Ожидаем завершения потомков (избавление от zombie-процессов)
    waitpid(son1, NULL, 0);
    waitpid(son2, NULL, 0);
    waitpid(son3, NULL, 0);

    return 0;
}