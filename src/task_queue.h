#include <pthread.h>
#include <stdbool.h>

typedef struct TaskQueue {
    int* items;
    int capacity;

    int head;
    int tail;
    int size;

    pthread_mutex_t mtx;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;

    bool queue_is_destroyed;
}TaskQueue;

TaskQueue *task_queue_init(int capacity);
void task_queue_enqueue(TaskQueue* queue, int item);
int task_queue_dequeue(TaskQueue* queue);

void task_queue_signal_destroy(TaskQueue* queue);
void task_queue_destroy(TaskQueue* queue);