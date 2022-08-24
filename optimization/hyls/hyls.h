#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include "localsearch.h"

#define AXVSIZE 15

void method(Individual InitSolution, const Task *inst_tasks_vt);
void method1(Individual *InitSolution, const Task *inst_tasks_vt);

