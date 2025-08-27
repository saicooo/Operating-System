

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Функция для циклических вычислений
int calculate(int var, char *type) {
    printf("\n--- Начало циклических вычислений для процесса %s ---\n", type);

    // Более заметная демонстрация конкуренции за процессорное время
    for (int i = 0; i < 10000000; i++) {
        if (i % 1000000 == 0) {
            printf("PID=%-7d | PPID=%-7d | iteration=%-10d | type=%-10s\n", getpid(), getppid(), i,
                   type);
        }
    }

    return var + 10;
}

int main() {
    // Отключаем буферизацию
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    int variable = 0;

    printf("--- Программа с циклическим вычислением запущена ---\n");

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Ошибка создания потомка\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Код потомка
        printf("Потомок запущен: PID=[%d], PPID=[%d], var=[%d]\n", getpid(), getppid(), variable);

        variable += 100; // Изменение в потомке не влияет на родителя (разные адресные пространства)
        variable = calculate(variable, "Потомок");

        printf("\nПотомок завершен: PID=[%d], PPID=[%d], var=[%d]\n", getpid(), getppid(),
               variable);
        exit(EXIT_SUCCESS);
    } else {
        // Код родителя
        printf("Родитель запущен: PID=[%d],  childPID=[%d], var=[%d]\n", getpid(), pid, variable);

        variable += 200; // Изменение в родителе не влияет на потомка (разные адресные пространства)
        variable = calculate(variable, "Родитель");

        // Ожидаем завершения потомка
        printf("\nРодитель ожидает завершения потомка с PID=[%d]...\n", pid);
        waitpid(pid, NULL, 0);

        printf("\nРодитель завершен: PID=[%d], var=[%d]\n\n", getpid(), variable);
    }

    printf("Программа завершена: PID=[%d]\n", getpid());
    
    return 0;
}