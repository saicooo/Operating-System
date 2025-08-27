/*
 * ======================================================================
 * Лабораторная работа 2, задание 5.3 (планирование FIFO)
 * ======================================================================
 *
 * ЗАДАЧА:
 * Исследование границ приоритетов для различных политик планирования
 * в Linux (SCHED_FIFO, SCHED_RR, SCHED_OTHER).
 *
 * ЧТО ДЕЛАЕТ ПРОГРАММА:
 * Программа определяет и выводит на экран минимальные и максимальные
 * значения приоритетов для трех основных политик планирования процессов
 * в Linux: FIFO (First-In-First-Out), RR (Round Robin) и OTHER (стандартная).
 * Эта информация необходима для корректной установки приоритетов при
 * экспериментах с планированием процессов.
 *
 * КОМПИЛЯЦИЯ И ЗАПУСК:
 * $ gcc limits.c -o limits
 * $ ./limits
 *
 * РЕЗУЛЬТАТЫ И ВЫВОДЫ:
 * - Отображает диапазоны допустимых значений приоритетов для каждой 
 *   политики планирования
 * - Помогает выбрать корректные значения приоритетов для экспериментов
 *   с процессами реального времени (FIFO, RR)
 * - Демонстрирует особенности различных политик планирования с точки
 *   зрения диапазона приоритетов
 */

#include <sched.h>
#include <stdio.h>

typedef struct {
    int min_border;
    int max_border;
} limits_t;

limits_t get_limits(int policy) {
    limits_t limits = {.min_border = sched_get_priority_min(policy),
                       .max_border = sched_get_priority_max(policy)};
    return limits;
}

int main() {
    limits_t limits[] = {get_limits(SCHED_FIFO), get_limits(SCHED_RR), get_limits(SCHED_OTHER)};
    printf("SCHED_FIFO : диапазон [%d;%d]\n", limits[0].min_border, limits[0].max_border);
    printf("SCHED_RR : диапазон [%d;%d]\n", limits[1].min_border, limits[1].max_border);
    printf("SCHED_OTHER: диапазон [%d;%d]\n", limits[2].min_border, limits[2].max_border);

    return 0;
}