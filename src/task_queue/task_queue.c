#include "../task_queue.h"

#include "../server.h"

#include <stdio.h>
#include <stdlib.h>

static inline bool task_queue_is_empty(TaskQueue *q) {
    return q->size == 0;
}

static inline bool task_queue_is_full(TaskQueue *q) {
    return q->size == q->capacity;
}

TaskQueue* task_queue_init(int capacity) {
    TaskQueue* queue = malloc(sizeof(TaskQueue));
    if (queue == NULL)
    {
        fprintf(stderr, "Couldn't create a task queue\n");
        return NULL;
    }
    
    queue->items = malloc(capacity * sizeof(*queue->items));
    if (queue->items == NULL)
    {
        fprintf(stderr, "Couldn't create a memory for tasks in task queue\n");
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    
    queue->queue_is_destroyed = false;
    
    pthread_mutex_init(&queue->mtx, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    return queue;
}

void task_queue_enqueue(TaskQueue* queue, int item) {
    pthread_mutex_lock(&queue->mtx);
    while (task_queue_is_full(queue) && !queue->queue_is_destroyed)
    {
        pthread_cond_wait(&queue->not_full,&queue->mtx);
    }

    if (queue->queue_is_destroyed)
    {
        pthread_mutex_unlock(&queue->mtx);
        return;
    }

    queue->items[queue->tail] = item;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;

    pthread_cond_broadcast(&queue->not_empty);
    pthread_mutex_unlock(&queue->mtx);
    return;
}

int task_queue_dequeue(TaskQueue* queue) {
    pthread_mutex_lock(&queue->mtx);
    while (task_queue_is_empty(queue) && !queue->queue_is_destroyed)
    {
        pthread_cond_wait(&queue->not_empty, &queue->mtx);
    }
    
    if (queue->queue_is_destroyed)
    {
        pthread_mutex_unlock(&queue->mtx);
        return -1;
    }

    int item = queue->items[queue->head];
    
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;

    pthread_cond_broadcast(&queue->not_full);
    pthread_mutex_unlock(&queue->mtx);
    return item;
}

void task_queue_signal_destroy(TaskQueue* queue) {
    if (queue == NULL) {
        return;
    }

    queue->queue_is_destroyed = true;
    
    pthread_mutex_lock(&queue->mtx);
    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);
    pthread_mutex_unlock(&queue->mtx);
}

void task_queue_destroy(TaskQueue* queue) {
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    
    pthread_mutex_destroy(&queue->mtx);
    
    free(queue->items);
    free(queue);
}
