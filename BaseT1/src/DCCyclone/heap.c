#include "heap.h"
#include <stdlib.h>

Queue* heap_create(unsigned long size) {
    Queue* h  = calloc(1, sizeof(Queue));
    h->array = calloc(size, sizeof(Process*));
    h->size  = size;
    h->used  = 0;
    return h;
}

void heap_destroy(Queue* target) {
    free(target->array);
    free(target);
}

static void heap_swap(Queue* target, unsigned long i, unsigned long j) {
    Process* tmp    = target->array[i];
    target->array[i] = target->array[j];
    target->array[j] = tmp;
}

// Retorna el índice de mayor prioridad EDF entre i y j, si hay empate retorna el índice de menor pid
static unsigned long heap_compare(Queue* target, unsigned long i, unsigned long j) {
    Process* a = target->array[i];
    Process* b = target->array[j];

    if (a->deadline != b->deadline)
        return (a->deadline < b->deadline) ? i : j;
    return (a->pid <= b->pid) ? i : j;
}

static void heap_sift_up(Queue* target, unsigned long idx) {
    while (idx > 0) {
        unsigned long parent = (idx - 1) / 2;
        if (heap_compare(target, idx, parent) == idx) {
            heap_swap(target, idx, parent);
            idx = parent;
        } else break;
    }
}

static void heap_sift_down(Queue* target, unsigned long idx) {
    while (1) {
        unsigned long best  = idx;
        unsigned long left  = 2 * idx + 1;
        unsigned long right = 2 * idx + 2;

        if (left  < target->used) best = heap_compare(target, best, left);
        if (right < target->used) best = heap_compare(target, best, right);

        if (best == idx) break;
        heap_swap(target, idx, best);
        idx = best;
    }
}

// Inserciones y eliminaciones

void heap_insert(Queue* target, Process* p) {
    if (target->used >= target->size) return;
    target->array[target->used++] = p;
    heap_sift_up(target, target->used - 1);
}

Process* heap_extract(Queue* target) {
    if (target->used == 0) return NULL;

    Process* top = target->array[0];
    target->array[0] = target->array[--target->used];
    heap_sift_down(target, 0);
    return top;
}

Process* heap_peek(Queue* target) {
    return target->used > 0 ? target->array[0] : NULL;
}

void heap_remove(Queue* target, Process* p) {
    for (unsigned long i = 0; i < target->used; i++) {
        if (target->array[i]->pid == p->pid) {
            target->array[i] = target->array[--target->used];
            heap_sift_up(target, i);
            heap_sift_down(target, i);
            return;
        }
    }
}