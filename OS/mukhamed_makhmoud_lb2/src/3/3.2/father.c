/*
 * ======================================================================
 * Лабораторная работа 2, задание 3.2 (родительский процесс)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Анализ значения, возвращаемого функцией wait(&status), демонстрация
 * связи wait() с SIGCHLD. Эксперимент, позволяющий родителю отслеживать
 * подмножество порожденных потомков используя функцию waitpid().
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Создает три дочерних процесса (son1, son2, son3), устанавливает
 * обработчик сигнала SIGCHLD для отслеживания изменений состояния
 * дочерних процессов. Затем отправляет сигналы SIGKILL и SIGINT
 * дочерним процессам и демонстрирует, как родитель может обрабатывать
 * информацию об их завершении через обработчик сигнала SIGCHLD.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc father.c -o father
 * $ gcc son1.c -o son1
 * $ gcc son2.c -o son2
 * $ gcc son3.c -o son3
 * $ ./father
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - При отправке сигнала SIGKILL процессу son1, он моментально завершился
 *   без возможности обработки сигнала.
 * - Процесс son2 получил сигнал SIGINT и успешно обработал его.
 * - Процесс son3 не получал сигналов от родителя и завершился самостоятельно
 *   с кодом 0 после истечения времени своей работы.
 * - Сигнал SIGCHLD вызвал обработчик в родительском процессе, который
 *   с помощью waitpid(-1, &status, WNOHANG) асинхронно собрал статусы всех
 *   завершившихся дочерних процессов, не блокируя основной поток выполнения.
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

// Функция для проверки изменения состояния дочерних процессов
void sigchld_handler(int sig) {
    int   status;
    pid_t pid;

    // Неблокирующий родительский процесс сбор информации обо всех завершившихся потомках
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("\n--- Процесс: PID=[%d], PPID=[%d] завершился самостоятельно: code=[%d] ---\n",
                   pid, getpid(), WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("\n--- Процесс: PID=[%d], PPID=[%d] завершился по сигналу: code=[%d] ---\n", pid,
                   getpid(), WTERMSIG(status));
        }
    }
}

int main() {
    // Отключаем буферизацию
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    char cmd[256];                    // Создаем переменную для вывода инфомрации в консоль
    signal(SIGCHLD, sigchld_handler); // Устанавливаем обработчик сигналов на сигнал SIGCHLD
                                      // (изменение состояния дочерних процессов)

    sprintf(cmd, "\n--- Информация о father ---\nPID=[%d], PPID=[%d]\n\n", getpid(), getppid());
    printf("%s", cmd);

    // Создание и запуск потомков
    int   status1;
    pid_t son1 = run_child("son1");

    int   status2;
    pid_t son2 = run_child("son2");

    int   status3;
    pid_t son3 = run_child("son3");

    sleep(1); // Ждем некоторое время (для корректного запуска потомков)

    sprintf(cmd,
            "echo '\n--- Вывод ps от процесса-отца (PID=%d) до отправки сигналов ---' && ps -l",
            getpid());
    system(cmd);
    printf("\n");

    // Отправка сигналов дочерним процессам
    kill(son1, SIGKILL); // Отправка принудительного завершения процессу son1
    kill(son2, SIGINT);  // Отправка прерывания процессу son2

    sprintf(cmd,
            "echo '\n--- Вывод ps от процесса-отца (PID=%d) после отправки сигналов ---' "
            "&& ps -l",
            getpid());
    system(cmd);

    sleep(5); // Даем время работы для дочерних процессов
}