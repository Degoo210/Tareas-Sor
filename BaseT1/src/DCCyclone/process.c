#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "process.h"

Process* process_create(char* name, int pid, int start, int burstTime, int burstQty, int ioTime, int deadline) {
    Process* new_p = malloc(sizeof(Process));
    
    strncpy(new_p->nombre, name, sizeof(new_p->nombre) - 1);

    new_p->estado    = NEW;

    new_p->nombre[sizeof(new_p->nombre) - 1] = '\0';
    new_p->pid       = pid;
    new_p->start     = start;
    new_p->burstTime = burstTime;
    new_p->burstQty  = burstQty;
    new_p->ioTime    = ioTime;
    new_p->deadline  = deadline;

    new_p->tlcpu           = -1;
    new_p->bursts_restantes = burstQty;
    new_p->burst_progreso  = 0;
    new_p->io_progreso      = 0;
    new_p->quantum_usado     = 0;
    new_p->inHigh          = false;

    new_p->interruptions = 0;
    new_p->turnaround    = 0;
    new_p->responseTime  = 0;
    new_p->waitingTime   = 0;
    new_p->readyWaitingTime = 0; 
    new_p->at_least_one_ejecution    = false;

    return new_p;
}

void process_destroy(Process* p) {
    free(p);
}

const char* process_state_str(ProcessState estado) {
    switch (estado) {
        case RUNNING:  return "RUNNING";
        case READY:    return "READY";
        case WAITING:  return "WAITING";
        case FINISHED: return "FINISHED";
        case DEAD:     return "DEAD";
        case NEW:     return "NEW";
        default:       return "UNKNOWN";
    }
}