#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "process.h"
#include "heap.h"

int compare_start(const void* a, const void* b) {
    Process* pa = *(Process**)a;
    Process* pb = *(Process**)b;
    if (pa->start != pb->start)
        return pa->start - pb->start;
    return pa->pid - pb->pid;
}

int compare_pid(const void* a, const void* b) {
    Process* pa = *(Process**)a;
    Process* pb = *(Process**)b;
    return pa->pid - pb->pid;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error al abrir archivo de entrada\n");
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        printf("Error al abrir archivo de salida\n");
        return 1;
    }

    int q, agingThreshold, nProcesses;

    fscanf(file, "%d", &q);
    fscanf(file, "%d", &agingThreshold);
    fscanf(file, "%d", &nProcesses);

    Process** processes = malloc(sizeof(Process*) * nProcesses);

    char name[20];
    int pid, start, burstTime, burstQty, ioTime, deadline;

    for (int i = 0; i < nProcesses; i++) {
        fscanf(file, "%s %d %d %d %d %d %d",
               name, &pid, &start, &burstTime,
               &burstQty, &ioTime, &deadline);

        processes[i] = process_create(name, pid, start, burstTime, burstQty, ioTime, deadline);
    }

    fclose(file);

    qsort(processes, nProcesses, sizeof(Process*), compare_start);

    int tick = 0;
    int next = 0;
    int done = 0;

    Queue* high = heap_create(nProcesses);
    Queue* low  = heap_create(nProcesses);

    Process* running = NULL;

    while (done < nProcesses) {

        // Paso 1: Llegadas
        while (next < nProcesses && processes[next]->start == tick) {
            processes[next]->estado = READY;
            processes[next]->inHigh = true;
            heap_insert(high, processes[next]);
            next++;
        }

        // Paso 2: Retorno de I/O
        for (int i = 0; i < nProcesses; i++) {
            Process* p = processes[i];
            if (p->estado == WAITING) {
                p->io_progreso++;
                if (p->io_progreso >= p->ioTime) {
                    p->io_progreso = 0;
                    p->estado = READY;
                    if (!p->inHigh) {
                        p->quantum_usado = 0;
                    }
                    p->inHigh = true;
                    heap_insert(high, p);
                }
            }
        }

        // Paso 3: Ascenso
        for (int i = (int)low->used - 1; i >= 0; i--) {
            Process* p = low->array[i];
            if (tick - p->tlcpu > agingThreshold) {
                p->quantum_usado = 0;
                p->inHigh = true;
                heap_remove(low, p);
                heap_insert(high, p);
            }
        }

        // Paso 4: Verificación de deadlines
        for (int i = 0; i < nProcesses; i++) {
            Process* p = processes[i];
            if ((p->estado == READY || p->estado == WAITING) && tick >= p->deadline) {
                p->turnaround  = tick - p->start;
                p->waitingTime = p->readyWaitingTime;
                p->estado = DEAD;
                done++;
                if (p->inHigh) heap_remove(high, p); else heap_remove(low, p);
            }
        }

        // Paso 5: Ordenamiento (Ya está ordenado por heap :D)

        // Paso 6: Ejecución
        if (running != NULL) {
            running->burst_progreso++;
            if (running->inHigh) {
                running->quantum_usado++;
            }

            // 6.1 Si termina su último burst y no hay ráfaga restante: cambiar a FINISHED
            if (running->burst_progreso >= running->burstTime && running->bursts_restantes == 1) {
                running->bursts_restantes--;
                running->tlcpu = tick;
                running->turnaround = tick - running->start;
                running->waitingTime = running->readyWaitingTime;
                running->estado = FINISHED;
                done++;
                running = NULL;
            }
            // 6.2 Si llega a su deadline absoluto: cambiar a DEAD
            else if (tick >= running->deadline) {
                running->interruptions++;
                running->tlcpu = tick;
                running->turnaround = tick - running->start;
                running->waitingTime = running->readyWaitingTime;
                running->estado = DEAD;
                done++;
                running = NULL;
            }
            // 6.3 Si termina una ráfaga (pero no es la última): cambiar a WAITING para I/O
            else if (running->burst_progreso >= running->burstTime) {
                running->bursts_restantes--;
                running->burst_progreso = 0;
                running->tlcpu = tick;
                running->estado = WAITING;
                running = NULL;
            }
            // 6.4 Si consume todo su quantum: cambiar a READY y descender a la cola Low si estaba en High
            else if (running->inHigh && running->quantum_usado >= q) {
                running->interruptions++;
                running->tlcpu = tick;
                running->estado = READY;
                running->inHigh = false;
                heap_insert(low, running);
                running = NULL;
            }
            // 6.5 Si hay un proceso con mayor prioridad en estado READY, pasar proceso en ejecución a estado READY y regresarlo a su cola de origen
            else {
                Process* top = heap_peek(high);
                bool preempt_high = (top != NULL && running->inHigh &&
                    (top->deadline < running->deadline ||
                    (top->deadline == running->deadline && top->pid < running->pid)));
                bool preempt_low = (!running->inHigh && high->used > 0);

                if (preempt_high || preempt_low) {
                    running->interruptions++;
                    running->tlcpu = tick;
                    running->estado = READY;
                    if (running->inHigh) heap_insert(high, running);
                    else heap_insert(low, running);
                    running = NULL;
                }
            }
        }

        // Paso 7: Dispatch
        if (running == NULL) {
            
            if (high->used > 0) {
                running = heap_extract(high);
                running->inHigh = true;
            } else if (low->used > 0) {
                running = heap_extract(low);
                running->inHigh = false;
            }
            if (running != NULL) {
                running->estado = RUNNING;
                if (!running->at_least_one_ejecution) {
                    running->responseTime = tick - running->start;
                    running->at_least_one_ejecution   = true;
                }
            }
        }

        for (int i = 0; i < nProcesses; i++) {
            Process* p = processes[i];
            if (p->estado == READY || p->estado == WAITING) {
                p->readyWaitingTime++;
            }
        }

        tick++;
    }

    qsort(processes, nProcesses, sizeof(Process*), compare_pid);

    for (int i = 0; i < nProcesses; i++) {
        Process* p = processes[i];
        fprintf(out, "%s,%d,%s,%d,%d,%d,%d\n",
                p->nombre,
                p->pid,
                process_state_str(p->estado),
                p->interruptions,
                p->turnaround,
                p->responseTime,
                p->waitingTime);
    }

    // Liberar memoria
    for (int i = 0; i < nProcesses; i++) {
        process_destroy(processes[i]);
    }
    free(processes);
    heap_destroy(high);
    heap_destroy(low);
    fclose(out);

    return 0;
};