#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "sort.h"

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Использование: %s <размер> <тип: 0-случайный, 1-обратный, 2-почти отсортированный> <потоки>\n", argv[0]);
        return 1;
    }

    int orig_n = atoi(argv[1]);
    int type = atoi(argv[2]);
    int threads = atoi(argv[3]);

    if (orig_n <= 0 || threads <= 0) {
        printf("Размер и количество потоков должны быть больше 0\n");
        return 1;
    }

    int n = 1;
    while (n < orig_n) {
        if (n > INT_MAX / 2) {
            printf("Размер массива слишком велик\n");
            return 1;
        }
        n *= 2;
    }

    int *original = malloc(n * sizeof(int));
    int *arr_seq = malloc(n * sizeof(int));
    int *arr_par = malloc(n * sizeof(int));

    if (!original || !arr_seq || !arr_par) {
        printf("Ошибка выделения памяти\n");
        return 1;
    }

    srand(time(NULL));

    if (type == 0) {
        for (int i = 0; i < orig_n; i++) {
            original[i] = rand() % 10000;
        }
        printf("Тест: Случайный массив, элементов: %d\n", orig_n);
    } else if (type == 1) {
        for (int i = 0; i < orig_n; i++) {
            original[i] = orig_n - i;
        }
        printf("Тест: Обратный порядок, элементов: %d\n", orig_n);
    } else {
        for (int i = 0; i < orig_n; i++) {
            original[i] = i;
        }
        for (int i = 0; i < orig_n / 100; i++) {
            int idx1 = rand() % orig_n;
            int idx2 = rand() % orig_n;
            int temp = original[idx1];
            original[idx1] = original[idx2];
            original[idx2] = temp;
        }
        printf("Тест: Почти отсортированный массив, элементов: %d\n", orig_n);
    }

    for (int i = orig_n; i < n; i++) {
        original[i] = INT_MAX;
    }

    memcpy(arr_seq, original, n * sizeof(int));
    double start = get_time_ms();
    sequential_sort(arr_seq, n);
    double time_seq = get_time_ms() - start;

    printf("Время исполнения (мс) последовательной сортировки: %.3f мс\n", time_seq);

    memcpy(arr_par, original, n * sizeof(int));
    start = get_time_ms();
    parallel_sort(arr_par, n, threads);
    double time_par = get_time_ms() - start;

    double speedup = time_seq / time_par;
    double efficiency = (speedup / threads) * 100;

    printf("Потоки | Время исполнения (мс) | Ускорение | Эффективность\n");
    printf("-----------------------------------------------------------\n");
    printf("%6d | %22.3f | %9.3f | %13.2f%%\n", threads, time_par, speedup, efficiency);

    int is_sorted = 1;
    for (int i = 1; i < orig_n; i++) {
        if (arr_par[i - 1] > arr_par[i]) {
            is_sorted = 0;
            break;
        }
    }

    if (!is_sorted) {
        printf("Ошибка: Массив не отсортирован!\n");
    } else if (memcmp(arr_seq, arr_par, n * sizeof(int)) != 0) {
        printf("Ошибка: Результаты параллельной и последовательной сортировки различаются!\n");
    }

    free(original);
    free(arr_seq);
    free(arr_par);
    return 0;
}
