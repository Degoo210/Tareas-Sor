#pragma once
#include <stdbool.h>
#include "process.h"

typedef struct {
    Process** array;
    unsigned long size;
    unsigned long used;
} Queue;

Queue*        heap_create(unsigned long size);
void          heap_destroy(Queue* target);
void          heap_insert(Queue* target, Process* p);
Process*      heap_extract(Queue* target);
Process*      heap_peek(Queue* target);
void          heap_remove(Queue* target, Process* p);