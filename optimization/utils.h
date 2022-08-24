#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "src/tinyxml2.h"
#include "time.h"

#define ARC_NUM 6633
#define NODE_NUM 2895
#define INF 1000000000.0
#define MAX_TASK_TAG_LENGTH 500
#define MAX_TASK_SEQ_LENGTH 200
#define MAX_NODE_TAG_LENGTH 500

extern int trav_cost[NODE_NUM+1][NODE_NUM+1];
extern int min_cost[NODE_NUM+1][NODE_NUM+1];
extern bool links[ARC_NUM+1][ARC_NUM+1];

typedef struct arc
{
    int head_node;
    int tail_node;
    int trav_cost;
    int IDindex;
} Arc;

// format: head ----> tail
typedef struct task
{
    int head_node;
    int tail_node;
    int demand;
    int serv_cost;
    int inverse;
    int vt;
    int IDindex;
} Task;

extern Arc roads[7000];
extern Task inst_tasks[301];

extern char path5[50];

void mod_dijkstra();
void save_min_cost();
void load_min_cost();
void select_tasks();
void detect_links();


void rand_perm(int *a, int num);
int rand_choose(int num);
void delete_element(int *a, int k);
void find_ele_positions(int *positions, int *a, int e);
int get_task_seq_total_cost(int *task_seq, const Task *inst_tasks);
void AssignArray(int *Array1, int *Array2);
void AssignSubArray(int *Array1, int k1, int k2, int *Array2);
void JoinArray(int *JointArray, int *Array);
void add_element(int *a, int e, int k);
void ReverseDirection(int *Array, int k1, int k2);
int FindTask(int a, int b, const Task *inst_tasks, int NO_Task);
int max(int *Array);
void rand_selection(int *id1, int *id2, int popsize);
int get_total_vio_load(int *route_seg_load);

extern int req_edge_num;
extern int task_num;
extern int capacity;
extern int DEPOT;
extern int terminal_duration;

// data strcuture for heuristic/meta-heuristic
typedef struct individual
{
    int Sequence[500]; // task id with depot inside
    int Assignment[300]; // when the representation is chromosome,this is used as the chromosome.
    int TotalCost;
    int Loads[101];
    int TotalVioLoad;
    double Fitness;
//    int start[101]; // for DCARP
} Individual;

void indi_route_converter(Individual *dst, Individual *src, const Task *inst_tasks);
void path_scanning(Individual *ps_indi, const Task *inst_tasks, const int *serve_mark);
void augment_merge(Individual *am_indi, const Task *inst_tasks);
int split(int *split_task_seq, int *one_task_seq, const Task *inst_tasks);
void FredericksonHeuristic(int *FHRoute, int *Route, const Task *inst_tasks);
void save_solution(Individual *indi, const Task *inst_tasks, char *path);
void check_solution_valid(Individual solution, const Task *inst_task);
int init();
void copy_individual(Individual *dest, Individual *src);
void clear_solution(Individual *solution);


int ILS(Individual *InitSolution, Individual *bestSolution, const Task *inst_tasks_vt);
void HyLS(Individual *InitSolution, Individual *bestSolution, const Task *inst_tasks_vt);
void LMA(const Task *inst_tasks, Individual *LMASolution);
void TSA(const Task *inst_tasks, Individual *TSASolution);
void MAENS(const Task *inst_tasks, Individual *MAENSolution);
void MASDC(Individual *best_individual, Task *inst_tasks, const int *stop, const int *remain_capacity);