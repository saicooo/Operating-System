/*
 * ======================================================================
 * Лабораторная работа 2, задание 2.3 (демонстрация функций exec)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Создание процессов с использованием различных функций семейства exec()
 * с разными параметрами, демонстрация влияния переменных окружения и
 * входных параметров на выполнение программ.
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Демонстрирует работу всех шести функций семейства exec:
 * - execl()  - с явным указанием аргументов
 * - execlp() - с поиском по PATH и явным указанием аргументов
 * - execle() - с явным указанием аргументов и переменных окружения
 * - execv()  - с передачей массива аргументов
 * - execvp() - с поиском по PATH и передачей массива аргументов
 * - execve() - с передачей массива аргументов и переменных окружения
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc exec_demo.c -o exec_demo
 * $ ./exec_demo
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Все функции семейства exec заменяют текущий образ процесса новым
 * - После успешного вызова exec код процесса после вызова не выполняется
 * - Различные функции семейства предоставляют гибкие способы передачи
 *   аргументов и переменных окружения новой программе
 * - Варианты с "p" автоматически ищут программу в каталогах из PATH
 * - Варианты с "e" позволяют задать собственное окружение для новой программы
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t create_child() {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Ошибка создания потомка");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        printf("Был создан потомок: PID=[%d], PPID=[%d]\n", getpid(), getppid());
    }

    return pid;
}

/*
execl - выполняет программу со списком аргументов:

Аргументы:
1) path - путь к исполняемому файлу
2) arg0, ... - список аргументов неопределенной длины, обычно - имя программы (argv_v[0])
3) NULL - маркер конца списка аргументов
*/
void execl_demo() {
    pid_t pid = create_child();

    if (pid == 0) {
        const char *path = "/bin/echo";
        const char *name = "echo";
        const char *text = "Hello, execl!";

        execl(path, name, text, NULL);
        fprintf(stderr, "Ошибка в execl");
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}

/*
execlp - как execl, но ищет программу в каталогах, указанных в переменной $PATH

Аргументы:
1) file - имя программы для поиска в $PATH
2) arg0, ... - список аргументов неопределенной длины, обычно - имя программы (argv_v[0])
3) NULL - маркер конца списка аргументов
*/
void execlp_demo() {
    pid_t pid = create_child();

    const char *file  = "ls";
    const char *flag  = "-l";
    const char *flag2 = "-a";

    if (pid == 0) {
        execlp(file, file, flag, flag2, NULL);
        fprintf(stderr, "Ошибка в execlp");
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}

/*
execle - как execl, но позволяет указать окружение для новой программы.

Аргументы:
1) path - путь к исполняемому файлу
2) arg0, ... - список аргументов неопределенной длины, обычно - имя программы (argv_v[0])
3) NULL - маркер конца списка аргументов
4) envp[] - массив указателей на строки окружения ("переменная окржения"="значение переменной")
*/
void execle_demo() {
    pid_t pid = create_child();

    const char *path   = "/usr/bin/env";
    const char *name   = "env";
    char *const envp[] = {"PATH=/bin:/usr/bin", "TERM=xterm", "CUSTOM_VAR=hello", NULL};

    if (pid == 0) {
        execle(path, name, NULL, envp);
        fprintf(stderr, "Ошибка в execle");
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}

/*
execv - выполняет программу с аргументами, переданными в виде массива

Аргументы:
1) path - полный путь к исполняемому файлу
2) argv[] - массив указателей на строку аргументов (первый аргумент - имя программы, последний -
NULL)
*/
void execv_demo(const char *path, char *const argv_v[]) {
    pid_t pid = create_child();

    if (pid == 0) {
        execv(path, argv_v);
        fprintf(stderr, "Ошибка в execv");
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}

/*
execvp - как execv, но ищет программу в каталогах, указанных в $PATH.

Аргументы:
1) file - имя программы для поиска в $PATH
2) argv[] - массив указателей на строку аргументов (первый аргумент - имя программы, последний -
NULL)
*/
void execvp_demo(const char *file, char *const argv_vp[]) {
    pid_t pid = create_child();

    if (pid == 0) {
        execvp(file, argv_vp);
        fprintf(stderr, "Ошибка в execvp");
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}

/*
execve - как execv, но позволяет указать переменные окружение для новой программы.

Аргументы:
1) path - полный путь к исполняемому файлу
2) argv[] - массив указателей на строки аргументов
3) envp[] - массив указателей на строки окружения
*/
void execve_demo(const char *path, char *const argv_ve[], char *const envp[]) {
    pid_t pid = create_child();

    if (pid == 0) {
        execve(path, argv_ve, envp);
        fprintf(stderr, "Ошибка в execve");
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}

int main() {
    // Отключаем буферизацию
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    printf("===== ДЕМОНСТРАЦИЯ ФУНКЦИЙ СЕМЕЙСТВА EXEC =====\n");

    // Пример для execl
    printf("\n--- Пример execl ---\n\n");
    execl_demo();

    // Пример для execlp
    printf("\n--- Пример execlp ---\n\n");
    execlp_demo();

    // Пример для execle
    printf("\n--- Пример execle ---\n\n");
    execle_demo();

    // Пример для execv
    printf("\n--- Пример execv ---\n\n");
    char *argv_v[] = {"echo", "Hello, execv!", NULL};
    execv_demo("/bin/echo", argv_v);

    // Пример для execvp
    printf("\n--- Пример execvp ---\n\n");
    char *argv_vp[] = {"ls", "-l", ".", NULL};
    execvp_demo("ls", argv_vp);

    // Пример для execve
    printf("\n--- Пример execve ---\n\n");
    char *argv_ve[] = {"env", NULL};
    char *env_ve[]  = {"PATH=/bin:/usr/bin", "TERM=xterm", "ANOTHER_VAR=world", NULL};
    execve_demo("/usr/bin/env", argv_ve, env_ve);

    printf("\n===== ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА =====\n");

    return 0;
}