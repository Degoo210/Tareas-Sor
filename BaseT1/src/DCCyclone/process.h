#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    RUNNING,
    READY,
    WAITING,
    FINISHED,
    DEAD,
    NEW
} ProcessState;

typedef struct Process{
    ProcessState estado;
    char nombre[20];
    int pid;
    int start;
    int burstTime;
    int burstQty;
    int ioTime;
    int deadline;
    int tlcpu;

    int  bursts_restantes;
    int  burst_progreso;   
    int  io_progreso;      
    int  quantum_usado;      
    bool inHigh;                // si está o no en queue high

    int interruptions;
    int turnaround;
    int responseTime;
    int waitingTime;
    int readyWaitingTime;
    bool at_least_one_ejecution; // si ya entró a la CPU antes
} Process;

const char* process_state_str(ProcessState estado);
Process* process_create(char* name, int pid, int start, int burstTime, int burstQty, int ioTime, int deadline);
void     process_destroy(Process* p);