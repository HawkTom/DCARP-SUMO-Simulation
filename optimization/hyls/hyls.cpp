#include "hyls.h"


int debug = 0;
int logflag = 0;


Individual best1_si_solution;
Individual best1_di_solution;
Individual best1_swap_solution;
Individual best1_cross_solution;

Individual IndiList[20];
int IndiListLength = 0;

double total_time = 0;
int best= INF;



void get_first_solution(Individual *solution, const Task *inst_tasks);
int update_global_best();
int update1_global_best();
void move_vt_to_first(Individual *dst, Individual *src, const Task *inst_tasks);

int ILS(Individual *InitSolution, Individual *bestSolution, const Task *inst_tasks_vt)
{
    clock_t start_t, finish_t;
    start_t = clock();
    double duration = 0.0;
    int five_second_flag = 1;

    int ILS_best = INF;
    while(1)
    {
        method1(InitSolution, inst_tasks_vt);
        if (InitSolution->TotalCost < ILS_best)
        {
            ILS_best = InitSolution->TotalCost;
            copy_individual(bestSolution, InitSolution);
        }

        finish_t = clock();
        duration = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
        if (duration > 5 && five_second_flag > 0)
        {
            // record;
            FILE *fp;
            fp = fopen(path5, "a");
            fprintf(fp, "ILS: %d\t", ILS_best);
            fclose(fp);
            five_second_flag = 0;
        }
        // printf("ILS: %d %f\n", ILS_best, duration);
        if (duration > terminal_duration) break;

        memset(InitSolution->Assignment, 0, sizeof(InitSolution->Assignment));
        for (int i =1; i <= InitSolution->Sequence[0]; i++)
        {
            if (InitSolution->Sequence[i] == 0) continue;
            InitSolution->Assignment[0] ++;
            InitSolution->Assignment[InitSolution->Assignment[0]] = InitSolution->Sequence[i];        
        }
        memset(InitSolution->Sequence, 0, sizeof(InitSolution->Sequence));
        memset(InitSolution->Loads, 0,  sizeof(InitSolution->Loads));
        InitSolution->TotalCost = 0;
        InitSolution->TotalVioLoad = 0;
        InitSolution->Fitness = 0;  

        int perm_index[250];
        rand_perm(perm_index, InitSolution->Assignment[0]);
        
        int one_seq[250];
        memcpy(one_seq, InitSolution->Assignment, sizeof(InitSolution->Assignment));

        for (int i = 1; i <= one_seq[0]; i++)
        {
            InitSolution->Assignment[i] = one_seq[perm_index[i]];
        }

        InitSolution->TotalCost = split(InitSolution->Sequence, InitSolution->Assignment, inst_tasks_vt);

        Individual tmpSolution;
        move_vt_to_first(&tmpSolution, InitSolution, inst_tasks_vt);
        copy_individual(InitSolution, &tmpSolution);
        finish_t = clock();
        duration = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
        // printf("ILS: %d %f\n", ILS_best, duration);
        if (duration > terminal_duration) break;
    }
    printf("ILS: %d\n", ILS_best);
    return ILS_best;
}

void HyLS(Individual *InitSolution, Individual *bestSolution, const Task *inst_tasks_vt)
{
    for (int i = 1; i < AXVSIZE; i++)
    {
        method1(InitSolution, inst_tasks_vt);
        // printf("%d %d \n", InitSolution->TotalCost, get_task_seq_total_cost(InitSolution->Sequence, inst_tasks_vt));
        if (InitSolution->TotalCost < best)
        {
            best = InitSolution->TotalCost;
            copy_individual(bestSolution, InitSolution);
        }
        copy_individual(InitSolution, &IndiList[i]);
        // printf("best: %d \n", best);
        if (i >= IndiListLength)
        {
            break;
        }
    }
    check_solution_valid(*bestSolution, inst_tasks);
    printf("HyLS complete. best: %d\n", best);
    // printf("HyLS complete ...\n");
}

void method1(Individual *InitSolution, const Task *inst_tasks_vt)
{
    copy_individual(&best1_si_solution, InitSolution);
    copy_individual(&best1_di_solution, InitSolution);
    copy_individual(&best1_swap_solution, InitSolution);
    copy_individual(&best1_cross_solution, InitSolution);

    int improves[5];
    int terminate_flag = 0;

    clock_t start_t, finish_t;
    start_t = clock();
    double duration = 0.0, duration1=0.0;
    
    while (1)
    {
        memset(improves, 0, sizeof(improves));

        // #pragma omp parallel sections
        {
            // #pragma omp section
            improves[1] = client1_single_insertion(InitSolution, inst_tasks_vt);
            if (improves[1] && IndiListLength <= AXVSIZE - 1)
            {
                IndiListLength ++;
                copy_individual(&IndiList[IndiListLength], &best1_si_solution);
            }
            
               
            // #pragma omp section
            improves[2] = client1_double_insertion(InitSolution, inst_tasks_vt);
            if (improves[2]  && IndiListLength <= AXVSIZE - 1)
            {
                IndiListLength ++;
                copy_individual(&IndiList[IndiListLength], &best1_di_solution);
            }

            // #pragma omp section
            improves[3] = client1_swap(InitSolution, inst_tasks_vt);
            if (improves[3]  && IndiListLength <= AXVSIZE - 1)
            {
                IndiListLength ++;
                copy_individual(&IndiList[IndiListLength], &best1_swap_solution);
            }

            // #pragma omp section
            improves[4] = client1_cross(InitSolution, inst_tasks_vt);
            if (improves[4]  && IndiListLength <= AXVSIZE - 1)
            {
                IndiListLength ++;
                copy_individual(&IndiList[IndiListLength], &best1_cross_solution);
            }
        }

        finish_t = clock();
        duration = (double)(finish_t - start_t) / CLOCKS_PER_SEC;
        terminate_flag = 0;
        for (int i = 1; i <= 4; i++)
        {
            // printf("%d\t", improves[i]);
            terminate_flag += improves[i];
        }
        // printf("\n");

        int flag = update1_global_best();

        switch (flag)
        {
        case 1:
            copy_individual(InitSolution, &best1_si_solution);
            break;
        case 2:
            copy_individual(InitSolution, &best1_di_solution);
            break;
        case 3:
            copy_individual(InitSolution, &best1_swap_solution);
            break;
        case 4:
            copy_individual(InitSolution, &best1_cross_solution);
            break;
        default:
            break;
        }

        if (terminate_flag == 0 || duration > terminal_duration)
        {
            break;
        }
        // printf("1 -> Log: %d %d\n", InitSolution->TotalCost, terminate_flag);
    }
    total_time += duration;
    // printf("1 -> Log: %d\n", InitSolution->TotalCost);
    
}



int update1_global_best()
{
    int cost[5];
    cost[1] = best1_si_solution.TotalCost;
    cost[2] = best1_di_solution.TotalCost;
    cost[3] = best1_swap_solution.TotalCost;
    cost[4] = best1_cross_solution.TotalCost;


    int i, k = 0;
    int mincost = INF;

    for (i = 1; i <= 4; i++)
    {
        // printf("%d \t", cost[i]);
        if (cost[i] < mincost)
        {
            mincost = cost[i];
            k = i;
        }
    }
    // printf("minvalue:%d, minindex:%d \n", mincost, k);
    return k;
}

void copy_individual(Individual *dest, Individual *src)
{
    memcpy(dest->Sequence, src->Sequence, sizeof(src->Sequence));
    memcpy(dest->Loads, src->Loads, sizeof(src->Loads));
    dest->TotalCost = src->TotalCost;
    dest->TotalVioLoad = src->TotalVioLoad;
    dest->Fitness = src->Fitness;
}

void move_vt_to_first(Individual *dst, Individual *src, const Task *inst_tasks)
{
    int i, load;
    load = 0;
    memset(dst->Sequence, 0, sizeof(int) * MAX_TASK_SEQ_LENGTH);
    memset(dst->Loads, 0, sizeof(int) * 50);
    dst->Sequence[0] = 1;
    dst->Sequence[1] = 0;
    for (i = 2; i <= src->Sequence[0]; i++)
    {
        if (src->Sequence[i] == 0)
        {
            dst->Sequence[0] ++;
            dst->Sequence[dst->Sequence[0]] = 0;
            dst->Loads[0] ++;
            dst->Loads[dst->Loads[0]] = load;
            load = 0;
            continue;
        }
        if (inst_tasks[src->Sequence[i]].vt > 0 && src->Sequence[i-1] != 0)
        {
            dst->Sequence[0] ++;
            dst->Sequence[dst->Sequence[0]] = 0;
            dst->Loads[0] ++;
            dst->Loads[dst->Loads[0]] = load;
            load = 0;
        }

        load += inst_tasks[src->Sequence[i]].demand;
        dst->Sequence[0] ++;
        dst->Sequence[dst->Sequence[0]] = src->Sequence[i];
    }
    dst->TotalCost = src->TotalCost;
}