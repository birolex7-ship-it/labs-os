#include <stdlib.h>
#include <pthread.h>
#include "sort.h"

static void compare(int *arr, int i, int j, int n) {
    if (j < n && arr[i] > arr[j]) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

static void merge(int *arr, int left, int right, int step, int n) {
    int double_step = step * 2;
    if (double_step < right - left) {
        merge(arr, left, right, double_step, n);
        merge(arr, left + step, right, double_step, n);
        for (int i = left + step; i < right - step; i += double_step) {
            compare(arr, i, i + step, n);
        }
    } else {
        compare(arr, left, left + step, n);
    }
}

static void recursive_sort(int *arr, int left, int right, int n) {
    if (right - left >= 1) {
        int middle = left + (right - left) / 2;
        recursive_sort(arr, left, middle, n);
        recursive_sort(arr, middle + 1, right, n);
        merge(arr, left, right, 1, n);
    }
}

void sequential_sort(int *arr, int n) {
    recursive_sort(arr, 0, n - 1, n);
}

static int max_threads = 1;
static int created_threads = 1;
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int *arr;
    int left;
    int right;
    int n;
} Task;

static void parallel_recursive_sort(int *arr, int left, int right, int n);

static void* thread_worker(void *arg) {
    Task *t = arg;
    parallel_recursive_sort(t->arr, t->left, t->right, t->n);
    free(t);
    return NULL;
}

static void parallel_recursive_sort(int *arr, int left, int right, int n) {
    if (right - left < 1) {
        return;
    }

    int middle = left + (right - left) / 2;

    pthread_mutex_lock(&thread_mutex);
    int can_create = (created_threads < max_threads);
    if (can_create) created_threads++;
    pthread_mutex_unlock(&thread_mutex);

    if (can_create) {
        Task *t = malloc(sizeof(Task));
        t->arr = arr;
        t->left = middle + 1;
        t->right = right;
        t->n = n;

        pthread_t tid;
        pthread_create(&tid, NULL, thread_worker, t);
        parallel_recursive_sort(arr, left, middle, n);
        pthread_join(tid, NULL);
    } else {
        parallel_recursive_sort(arr, left, middle, n);
        parallel_recursive_sort(arr, middle + 1, right, n);
    }

    merge(arr, left, right, 1, n);
}

void parallel_sort(int *arr, int n, int threads) {
    max_threads = threads;
    created_threads = 1;
    parallel_recursive_sort(arr, 0, n - 1, n);
}
