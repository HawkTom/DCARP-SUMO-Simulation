#include "localsearch.h"
void check_demand(lns_route curr_solution, const Task *inst_tasks);

int check_cost1(lns_route curr_solution, const Task *inst_tasks)
{
    int i;
    int cost = 0, tmp_cost;
    for (i = 1; i <= curr_solution.Route[0][0]; i++)
    {
        tmp_cost = get_task_seq_total_cost(curr_solution.Route[i], inst_tasks);
        printf("%d \t", tmp_cost);
        cost += tmp_cost;
    }
    printf("\n");
    // printf("%d %d \n", cost, curr_solution.total_cost);
    if (cost != curr_solution.total_cost)
    {
        printf("%d %d cost error. \n", cost, curr_solution.total_cost);
        return 0;
    }
    return 1;
}

void check_demand(lns_route curr_solution, const Task *inst_tasks)
{
    int i, j;
    int cost = 0, tmp_load = 0;
    for (i = 1; i <= curr_solution.Route[0][0]; i++)
    {   
        tmp_load = 0;
        for (j = 1; j <= curr_solution.Route[i][0]; j++)
        {
            tmp_load += inst_tasks[curr_solution.Route[i][j]].demand;
        }
        if (tmp_load != curr_solution.loads[i])
        {
            printf(" load error \n");
        }
    }
}

int client1_single_insertion(Individual *indi, const Task *inst_tasks)
{

    int i, j, k;
    lns_route curr_solution, next_solution;

    char type[30];

    strcpy(type, "Single Insertion");

    int Positions[101];
    find_ele_positions(Positions, indi->Sequence, 0);
    curr_solution.Route[0][0] = Positions[0] - 1;
    curr_solution.loads[0] = Positions[0] - 1;
    for (i = 1; i < Positions[0]; i++)
    {
        AssignSubArray(indi->Sequence, Positions[i], Positions[i+1], curr_solution.Route[i]); // Route[i]: 0 x x x x 0
        curr_solution.loads[i] = 0;
        for (j = Positions[i]+1; j < Positions[i+1]; j++)
        {
            curr_solution.loads[i] += inst_tasks[indi->Sequence[j]].demand;
        }
    }
    curr_solution.fitness = indi->TotalCost + LAMBDA * indi->TotalVioLoad;;
    curr_solution.total_vio_loads = indi->TotalVioLoad;
    curr_solution.total_cost = indi->TotalCost;

    int u, pos_u, trip_u, v, pos_v, trip_v, x, y, inv_u;
    int improve = 0;
    int global_improve = 0;

    // check_demand(curr_solution, inst_tasks);
    while (1)
    {
        next_solution = curr_solution;
        for (trip_u = 1; trip_u <= curr_solution.Route[0][0]; trip_u ++)
        {
            for (pos_u = 2; pos_u < curr_solution.Route[trip_u][0]; pos_u++)
            {
                u = curr_solution.Route[trip_u][pos_u];

                for (trip_v = trip_u; trip_v <= curr_solution.Route[0][0]; trip_v ++)
                {
                    for (pos_v = pos_u + 1; pos_v < curr_solution.Route[trip_v][0]; pos_v ++)
                    {
                        // if (trip_u == 1 && trip_v == 4 && pos_u == 4 && pos_v == 5)
                        // {
                        //     int a = 1;
                        // }

                        v = curr_solution.Route[trip_v][pos_v];

                        improve = lma_single_insertion(&curr_solution, &next_solution, u, v, trip_u, trip_v, pos_u, pos_v, inst_tasks);
                        // check_demand(curr_solution, inst_tasks);
                        
                        if (improve){
//                            printf("single insertion \n");
                            goto new_step;
                        }
                        
                    }
                }
            }
        }


    new_step:
        if (improve)
        {
            // update to the server
            update_global_best_solution(&curr_solution, &best1_si_solution, inst_tasks, type);
            global_improve = 1;
            improve = 0;
            continue;
        } else {
            break;
        }
    }



    update_global_best_solution(&curr_solution, &best1_si_solution, inst_tasks, type);
    
    Individual mted_child;
    // route -> sequence
    mted_child.Sequence[0] = 1;
    k = 0;
    for (i = 1; i <= curr_solution.Route[0][0]; i++)
    {
        if (curr_solution.Route[i][0] > 2)
        {
            mted_child.Sequence[0] --;
            JoinArray(mted_child.Sequence, curr_solution.Route[i]);
            k ++;
            mted_child.Loads[k] = curr_solution.loads[i];
        }
    }
    mted_child.Loads[0] = k;
    mted_child.TotalCost = curr_solution.total_cost;
    mted_child.TotalVioLoad = curr_solution.total_vio_loads;

    if ( mted_child.TotalCost != get_task_seq_total_cost(mted_child.Sequence, inst_tasks))
    {
        printf("101: lmsals total cost error \n");
        exit(0);
    }
    return global_improve;

}


int client1_double_insertion(Individual *indi, const Task *inst_tasks)
{

    int i, j, k;
    lns_route curr_solution, next_solution;

    char type[30];

    strcpy(type, "Double Insertion");

    int Positions[101];
    find_ele_positions(Positions, indi->Sequence, 0);
    curr_solution.Route[0][0] = Positions[0] - 1;
    curr_solution.loads[0] = Positions[0] - 1;
    for (i = 1; i < Positions[0]; i++)
    {
        AssignSubArray(indi->Sequence, Positions[i], Positions[i+1], curr_solution.Route[i]); // Route[i]: 0 x x x x 0
        curr_solution.loads[i] = 0;
        for (j = Positions[i]+1; j < Positions[i+1]; j++)
        {
            curr_solution.loads[i] += inst_tasks[indi->Sequence[j]].demand;
        }
    }
    curr_solution.fitness = indi->TotalCost + LAMBDA * indi->TotalVioLoad;;
    curr_solution.total_vio_loads = indi->TotalVioLoad;
    curr_solution.total_cost = indi->TotalCost;

    int u, pos_u, trip_u, v, pos_v, trip_v, x, y, inv_u;
    int improve = 0;
    int global_improve = 0;

    
    while (1)
    {
        next_solution = curr_solution;
        for (trip_u = 1; trip_u <= curr_solution.Route[0][0]; trip_u ++)
        {
            for (pos_u = 2; pos_u < curr_solution.Route[trip_u][0]; pos_u++)
            {
                u = curr_solution.Route[trip_u][pos_u];

                for (trip_v = trip_u; trip_v <= curr_solution.Route[0][0]; trip_v ++)
                {
                    for (pos_v = pos_u + 1; pos_v < curr_solution.Route[trip_v][0]; pos_v ++)
                    {

                        v = curr_solution.Route[trip_v][pos_v];

                        if (trip_u == trip_v && pos_v - pos_u > 1 || trip_v != trip_u)
                        {

                            x = curr_solution.Route[trip_u][pos_u+1];
                            if (trip_v != trip_u && x != 0)
                            {
                                improve = lma_double_insertion(&curr_solution, &next_solution, u, x, v, trip_u, trip_v, pos_u, pos_v, inst_tasks);
                                // check_cost1(curr_solution, inst_tasks);
                                // check_demand(curr_solution, inst_tasks);
                                if (improve){
//                                    printf("double insertion \n");
                                    goto new_step;
                                }
                            }
                        }



                    }
                }
            }
        }


    new_step:
        if (improve)
        {
            // update to the server
            update_global_best_solution(&curr_solution, &best1_di_solution, inst_tasks, type);
            global_improve = 1;
            improve = 0;
            continue;
            // break;
        } else {
            break;
        }
    }


    update_global_best_solution(&curr_solution, &best1_di_solution, inst_tasks, type);
    
    Individual mted_child;
    // route -> sequence
    mted_child.Sequence[0] = 1;
    k = 0;
    for (i = 1; i <= curr_solution.Route[0][0]; i++)
    {
        if (curr_solution.Route[i][0] > 2)
        {
            mted_child.Sequence[0] --;
            JoinArray(mted_child.Sequence, curr_solution.Route[i]);
            k ++;
            mted_child.Loads[k] = curr_solution.loads[i];
        }
    }
    mted_child.Loads[0] = k;
    mted_child.TotalCost = curr_solution.total_cost;
    mted_child.TotalVioLoad = curr_solution.total_vio_loads;

    if ( mted_child.TotalCost != get_task_seq_total_cost(mted_child.Sequence, inst_tasks))
    {
        printf("214: lmsals total cost error \n");
        // longjmp(buf, 2);
        exit(0);
    }
    return global_improve;

}


int client1_swap(Individual *indi, const Task *inst_tasks)
{

    int i, j, k;
    lns_route curr_solution, next_solution;

    char type[30];

    strcpy(type, "SWAP");

    int Positions[101];
    find_ele_positions(Positions, indi->Sequence, 0);
    curr_solution.Route[0][0] = Positions[0] - 1;
    curr_solution.loads[0] = Positions[0] - 1;
    for (i = 1; i < Positions[0]; i++)
    {
        AssignSubArray(indi->Sequence, Positions[i], Positions[i+1], curr_solution.Route[i]); // Route[i]: 0 x x x x 0
        curr_solution.loads[i] = 0;
        for (j = Positions[i]+1; j < Positions[i+1]; j++)
        {
            curr_solution.loads[i] += inst_tasks[indi->Sequence[j]].demand;
        }
    }
    curr_solution.fitness = indi->TotalCost + LAMBDA * indi->TotalVioLoad;;
    curr_solution.total_vio_loads = indi->TotalVioLoad;
    curr_solution.total_cost = indi->TotalCost;

    int u, pos_u, trip_u, v, pos_v, trip_v, x, y, inv_u;
    int improve = 0;
    int global_improve = 0;

    
    while (1)
    {
        next_solution = curr_solution;
        for (trip_u = 1; trip_u <= curr_solution.Route[0][0]; trip_u ++)
        {
            for (pos_u = 2; pos_u < curr_solution.Route[trip_u][0]; pos_u++)
            {
                u = curr_solution.Route[trip_u][pos_u];

                for (trip_v = trip_u; trip_v <= curr_solution.Route[0][0]; trip_v ++)
                {
                    for (pos_v = pos_u + 1; pos_v < curr_solution.Route[trip_v][0]; pos_v ++)
                    {

                        v = curr_solution.Route[trip_v][pos_v];

                        if (trip_u == trip_v && pos_v - pos_u > 1 || trip_v != trip_u)
                        {
                            improve = lma_swap(&curr_solution, &next_solution, u, v, trip_u, trip_v, pos_u, pos_v, inst_tasks);
                            // check_cost1(curr_solution, inst_tasks);
                            // check_demand(curr_solution, inst_tasks);;
                            if (improve){
//                                printf("swap \n");
                                goto new_step;
                            }
                        }

                    }
                }
            }
        }


    new_step:
        if (improve)
        {
            // update to the server
            update_global_best_solution(&curr_solution, &best1_swap_solution, inst_tasks, type);
            global_improve = 1;
            improve = 0;
            continue;
            // break;
        } else {
            break;
        }
    }


    update_global_best_solution(&curr_solution, &best1_swap_solution, inst_tasks, type);
    
    Individual mted_child;
    // route -> sequence
    mted_child.Sequence[0] = 1;
    k = 0;
    for (i = 1; i <= curr_solution.Route[0][0]; i++)
    {
        if (curr_solution.Route[i][0] > 2)
        {
            mted_child.Sequence[0] --;
            JoinArray(mted_child.Sequence, curr_solution.Route[i]);
            k ++;
            mted_child.Loads[k] = curr_solution.loads[i];
        }
    }
    mted_child.Loads[0] = k;
    mted_child.TotalCost = curr_solution.total_cost;
    mted_child.TotalVioLoad = curr_solution.total_vio_loads;

    if ( mted_child.TotalCost != get_task_seq_total_cost(mted_child.Sequence, inst_tasks))
    {
        printf("320: lmsals total cost error \n");
        // longjmp(buf, 2);
        exit(0);
    }

    return global_improve;
}

int client1_cross(Individual *indi, const Task *inst_tasks)
{
    int i, j, k;
    lns_route curr_solution, next_solution;

    char type[30];

    strcpy(type, "Cross");

    int Positions[101];
    find_ele_positions(Positions, indi->Sequence, 0);
    curr_solution.Route[0][0] = Positions[0] - 1;
    curr_solution.loads[0] = Positions[0] - 1;
    for (i = 1; i < Positions[0]; i++)
    {
        AssignSubArray(indi->Sequence, Positions[i], Positions[i+1], curr_solution.Route[i]); // Route[i]: 0 x x x x 0
        curr_solution.loads[i] = 0;
        for (j = Positions[i]+1; j < Positions[i+1]; j++)
        {
            curr_solution.loads[i] += inst_tasks[indi->Sequence[j]].demand;
        }
    }
    curr_solution.fitness = indi->TotalCost + LAMBDA * indi->TotalVioLoad;;
    curr_solution.total_vio_loads = indi->TotalVioLoad;
    curr_solution.total_cost = indi->TotalCost;

    int u, pos_u, trip_u, v, pos_v, trip_v, x, y, inv_u;
    int improve = 0;
    int global_improve = 0;

    
    while (1)
    {
        next_solution = curr_solution;
        for (trip_u = 1; trip_u <= curr_solution.Route[0][0]; trip_u ++)
        {
            for (pos_u = 2; pos_u < curr_solution.Route[trip_u][0]; pos_u++)
            {
                u = curr_solution.Route[trip_u][pos_u];

                for (trip_v = trip_u; trip_v <= curr_solution.Route[0][0]; trip_v ++)
                {
                    // printf("%d, %d ||", curr_solution.Route[trip_u][0],  curr_solution.Route[trip_v][0]);
                    for (pos_v = 2; pos_v < curr_solution.Route[trip_v][0]; pos_v ++)
                    {

                        // printf("pos: %d %d\n", pos_u, pos_v);
                        v = curr_solution.Route[trip_v][pos_v];

                        x = curr_solution.Route[trip_u][pos_u+1];

                        y = curr_solution.Route[trip_v][pos_v+1];
                        // printf("u: %d x:%d v:%d y:%d\n", u,x,v,y);

                        // printf("trip: %d %d\n", trip_u, trip_v);
                        // if (pos_u == 2 && pos_v == 2 && trip_u == 1 && trip_v == 4)
                        // {
                        //     printf("stop\n");
                        // }

                        if (trip_v != trip_u && x != 0 && y != 0)
                        {
                            improve = lma_cross(&curr_solution, &next_solution, u, v, trip_u, trip_v, pos_u, pos_v, inst_tasks);
                            // check_demand(curr_solution, inst_tasks);
                            if (improve){
                                goto new_step;
                            }
                        }

                    }
                }
            }
        }


    new_step:
        if (improve)
        {
            // update to the server
            update_global_best_solution(&curr_solution, &best1_cross_solution, inst_tasks, type);
            global_improve = 1;
            improve = 0;
            continue;
        } else {
            break;
        }
    }

    // printf("xxxx\n");
    update_global_best_solution(&curr_solution, &best1_cross_solution, inst_tasks, type);
    
    Individual mted_child;
    // route -> sequence
    mted_child.Sequence[0] = 1;
    k = 0;
    for (i = 1; i <= curr_solution.Route[0][0]; i++)
    {
        if (curr_solution.Route[i][0] > 2)
        {
            mted_child.Sequence[0] --;
            JoinArray(mted_child.Sequence, curr_solution.Route[i]);
            k ++;
            mted_child.Loads[k] = curr_solution.loads[i];
        }
    }
    mted_child.Loads[0] = k;
    mted_child.TotalCost = curr_solution.total_cost;
    mted_child.TotalVioLoad = curr_solution.total_vio_loads;

    if ( mted_child.TotalCost != get_task_seq_total_cost(mted_child.Sequence, inst_tasks))
    {
        printf("553: lmsals total cost error \n");
        // longjmp(buf, 2);
        exit(0);
    }
    if ( best1_cross_solution.Sequence[0] == 0)
    {
        printf("564: lmsals total cost error \n");
        // longjmp(buf, 2);
        exit(0);
    }
    return global_improve;

}
void update_global_best_solution(lns_route *route_solution, Individual *global_solution, const Task *inst_tasks, const char *type)
{
    memset(global_solution->Sequence, 0, sizeof(int)*MAX_TASK_SEQ_LENGTH);
    memset(global_solution->Loads, 0, sizeof(int)*101);
    global_solution->Sequence[0] = 0;
    int i, j, load = 0;
    for (i = 1; i <= route_solution->Route[0][0]; i++)
    {
        if (route_solution->Route[i][0] <= 2) continue;
        for (j = 1; j < route_solution->Route[i][0]; j++)
        {
            if( inst_tasks[route_solution->Route[i][j]].vt > 0 && route_solution->Route[i][j-1] != 0)
            {
                global_solution->Sequence[0] ++;
                global_solution->Sequence[global_solution->Sequence[0]] = 0;
                global_solution->Sequence[0] ++;
                global_solution->Sequence[global_solution->Sequence[0]] = route_solution->Route[i][j];
                continue;
            }
            global_solution->Sequence[0] ++;
            global_solution->Sequence[global_solution->Sequence[0]] = route_solution->Route[i][j];
        }
    }
    global_solution->Sequence[0] ++;
    global_solution->Sequence[global_solution->Sequence[0]] = 0;


    global_solution->Loads[0] = -1;
    for (i = 1; i <= global_solution->Sequence[0]; i++)
    {
        if (global_solution->Sequence[i] == 0)
        {
            global_solution->Loads[0] ++;
            global_solution->Loads[global_solution->Loads[0]] = load;
            load = 0;            
            continue;
        }
        load += inst_tasks[global_solution->Sequence[i]].demand; 
    }


    global_solution->TotalCost = route_solution->total_cost;
    global_solution->TotalVioLoad = route_solution->total_vio_loads;
    int cost = get_task_seq_total_cost(global_solution->Sequence, inst_tasks);
    if (cost != global_solution->TotalCost)
    {
        printf("616: cost error\n");
    }

    if (logflag){
        printf("%s: %d\n", type, global_solution->TotalCost);
    }

    route_solution->Route[0][0] = 1;
    route_solution->Route[1][0] = 1;
    route_solution->Route[1][1] = 0;

    for (i = 2; i <= global_solution->Sequence[0]; i++)
    {
        if (global_solution->Sequence[i] == 0)
        {
            route_solution->Route[route_solution->Route[0][0]][0] ++;
            route_solution->Route[route_solution->Route[0][0]][route_solution->Route[route_solution->Route[0][0]][0]] = 0;
            route_solution->Route[0][0] ++;
            route_solution->Route[route_solution->Route[0][0]][0] = 1;
            route_solution->Route[route_solution->Route[0][0]][1] = 0;
            continue;
        }
        route_solution->Route[route_solution->Route[0][0]][0] ++;
        route_solution->Route[route_solution->Route[0][0]][route_solution->Route[route_solution->Route[0][0]][0]] = global_solution->Sequence[i];
    }
    route_solution->Route[0][0] --;
    memcpy(route_solution->loads, global_solution->Loads, sizeof(global_solution->Loads));
    
}

int lma_single_insertion(lns_route *curr_solution, lns_route *next_solution, int u, int v, int trip_u, int trip_v, int pos_u, int pos_v, const Task *inst_tasks)
{
    next_solution->loads[trip_u] -= inst_tasks[u].demand;
    next_solution->loads[trip_v] += inst_tasks[u].demand;


    if (curr_solution->loads[trip_u] > capacity)
        next_solution->total_vio_loads -= curr_solution->loads[trip_u] - capacity;
    if (curr_solution->loads[trip_v] > capacity)
        next_solution->total_vio_loads -= curr_solution->loads[trip_v] - capacity;

    if (next_solution->loads[trip_u] > capacity)
        next_solution->total_vio_loads += next_solution->loads[trip_u] - capacity;
    if (next_solution->loads[trip_v] > capacity)
        next_solution->total_vio_loads += next_solution->loads[trip_v] - capacity;



    int flag;
    if (pos_v == 2) // first task
    {
        flag = 1;
        next_solution->total_cost = curr_solution->total_cost
                                    - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                    - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[u].head_node]
                                    + min_cost[inst_tasks[u].tail_node][inst_tasks[v].head_node];

    }
    else
    {
        flag = 0;
        next_solution->total_cost = curr_solution->total_cost
                                    - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                    - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                    + min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                    + min_cost[inst_tasks[v].tail_node][inst_tasks[u].head_node];
                                    
    }

    next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
    if (next_solution->fitness < curr_solution->fitness)
    {
        // printf("-----\n");
        // check_cost1(*curr_solution, inst_tasks);

        curr_solution->total_cost = next_solution->total_cost;
        curr_solution->total_vio_loads = next_solution->total_vio_loads;
        curr_solution->fitness = next_solution->fitness;
        curr_solution->loads[trip_u] = next_solution->loads[trip_u];
        curr_solution->loads[trip_v] = next_solution->loads[trip_v];
        

        if (flag) {
            add_element(curr_solution->Route[trip_v], u, pos_v);
        } else {
            add_element(curr_solution->Route[trip_v], u, pos_v+1);
        }
        delete_element(curr_solution->Route[trip_u], pos_u);
        // check_cost1(*curr_solution, inst_tasks);
        check_demand(*curr_solution, inst_tasks);
        return 1;
    }

    int inv_u = inst_tasks[u].inverse;
    if (inv_u > 0)
    {
        if (pos_v == 2) // first task
        {
            flag = 1;
            next_solution->total_cost = curr_solution->total_cost
                                        - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                        - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                        - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[inv_u].head_node]
                                        + min_cost[inst_tasks[inv_u].tail_node][inst_tasks[v].head_node];

        }
        else
        {
            flag = 0;
            next_solution->total_cost = curr_solution->total_cost
                                        - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                        - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                        - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                        + min_cost[inst_tasks[v].tail_node][inst_tasks[inv_u].head_node]
                                        + min_cost[inst_tasks[inv_u].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];
        }
        next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;

        if (next_solution->fitness < curr_solution->fitness)
        {
            curr_solution->total_cost = next_solution->total_cost;
            curr_solution->total_vio_loads = next_solution->total_vio_loads;
            curr_solution->fitness = next_solution->fitness;
            curr_solution->loads[trip_u] = next_solution->loads[trip_u];
            curr_solution->loads[trip_v] = next_solution->loads[trip_v];

            if (flag) {
                add_element(curr_solution->Route[trip_v], inv_u, pos_v);
            } else {
                add_element(curr_solution->Route[trip_v], inv_u, pos_v + 1);
            }
            delete_element(curr_solution->Route[trip_u], pos_u);
            // check_cost1(*curr_solution, inst_tasks);
            return 1;
        }
    }


    next_solution->total_cost = curr_solution->total_cost;
    next_solution->total_vio_loads = curr_solution->total_vio_loads;
    next_solution->fitness = curr_solution->fitness;
    next_solution->loads[trip_u] = curr_solution->loads[trip_u];
    next_solution->loads[trip_v] = curr_solution->loads[trip_v];

    return 0;
}

int lma_double_insertion(lns_route *curr_solution, lns_route *next_solution, int u, int x, int v, int trip_u, int trip_v, int pos_u, int pos_v, const Task *inst_tasks)
{

    next_solution->loads[trip_u] -= inst_tasks[u].demand + inst_tasks[x].demand;
    next_solution->loads[trip_v] += inst_tasks[u].demand + inst_tasks[x].demand;

    if (curr_solution->loads[trip_u] > capacity)
        next_solution->total_vio_loads -= curr_solution->loads[trip_u] - capacity;
    if (curr_solution->loads[trip_v] > capacity)
        next_solution->total_vio_loads -= curr_solution->loads[trip_v] - capacity;

    if (next_solution->loads[trip_u] > capacity)
        next_solution->total_vio_loads += next_solution->loads[trip_u] - capacity;
    if (next_solution->loads[trip_v] > capacity)
        next_solution->total_vio_loads += next_solution->loads[trip_v] - capacity;

    int flag;
    if (pos_v == 2)
    {
        flag = 1;
        next_solution->total_cost = curr_solution->total_cost
                                    - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                    - min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                    - min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                    - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[u].head_node]
                                    + min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                    + min_cost[inst_tasks[x].tail_node][inst_tasks[v].head_node];
    } else {
        flag = 0;
        next_solution->total_cost = curr_solution->total_cost
                                    - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                    - min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                    - min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                    - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                    + min_cost[inst_tasks[v].tail_node][inst_tasks[u].head_node]
                                    + min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                    + min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];
    }
    next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
    if (next_solution->fitness < curr_solution->fitness)
    {
        curr_solution->total_cost = next_solution->total_cost;
        curr_solution->total_vio_loads = next_solution->total_vio_loads;
        curr_solution->fitness = next_solution->fitness;
        curr_solution->loads[trip_u] = next_solution->loads[trip_u];
        curr_solution->loads[trip_v] = next_solution->loads[trip_v];
        if (flag) {
            add_element(curr_solution->Route[trip_v], x, pos_v);
            add_element(curr_solution->Route[trip_v], u, pos_v);
        } else {
            add_element(curr_solution->Route[trip_v], u, pos_v + 1);
            add_element(curr_solution->Route[trip_v], x, pos_v + 2);
        }

        delete_element(curr_solution->Route[trip_u], pos_u+1);
        delete_element(curr_solution->Route[trip_u], pos_u);
        return 1;
    }

    int inv_u = inst_tasks[u].inverse;
    if (inv_u > 0)
    {
        if (pos_v == 2)
        {
            flag = 1;
            next_solution->total_cost = curr_solution->total_cost
                                        - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                        - min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                        - min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[inv_u].head_node]
                                        + min_cost[inst_tasks[inv_u].tail_node][inst_tasks[x].head_node]
                                        + min_cost[inst_tasks[x].tail_node][inst_tasks[v].head_node];
        } else {
            flag = 0;
            next_solution->total_cost = curr_solution->total_cost
                                        - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                        - min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                        - min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                        + min_cost[inst_tasks[v].tail_node][inst_tasks[inv_u].head_node]
                                        + min_cost[inst_tasks[inv_u].tail_node][inst_tasks[x].head_node]
                                        + min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];
        }
        next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
        if (next_solution->fitness < curr_solution->fitness)
        {
            curr_solution->total_cost = next_solution->total_cost;
            curr_solution->total_vio_loads = next_solution->total_vio_loads;
            curr_solution->fitness = next_solution->fitness;
            curr_solution->loads[trip_u] = next_solution->loads[trip_u];
            curr_solution->loads[trip_v] = next_solution->loads[trip_v];
            if (flag) {
                add_element(curr_solution->Route[trip_v], x, pos_v);
                add_element(curr_solution->Route[trip_v], inv_u, pos_v);
            } else {
                add_element(curr_solution->Route[trip_v], inv_u, pos_v + 1);
                add_element(curr_solution->Route[trip_v], x, pos_v + 2);
            }

            delete_element(curr_solution->Route[trip_u], pos_u+1);
            delete_element(curr_solution->Route[trip_u], pos_u);
            return 1;
        }
    }


    int inv_x = inst_tasks[x].inverse;
    if (inv_x > 0)
    {
        if (pos_v == 2)
        {
            flag = 1;
            next_solution->total_cost = curr_solution->total_cost
                                        - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                        - min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                        - min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[u].head_node]
                                        + min_cost[inst_tasks[u].tail_node][inst_tasks[inv_x].head_node]
                                        + min_cost[inst_tasks[inv_x].tail_node][inst_tasks[v].head_node];
        } else {
            flag = 0;
            next_solution->total_cost = curr_solution->total_cost
                                        - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                        - min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                        - min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                        + min_cost[inst_tasks[v].tail_node][inst_tasks[u].head_node]
                                        + min_cost[inst_tasks[u].tail_node][inst_tasks[inv_x].head_node]
                                        + min_cost[inst_tasks[inv_x].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];
        }
        next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
        if (next_solution->fitness < curr_solution->fitness)
        {
            curr_solution->total_cost = next_solution->total_cost;
            curr_solution->total_vio_loads = next_solution->total_vio_loads;
            curr_solution->fitness = next_solution->fitness;
            curr_solution->loads[trip_u] = next_solution->loads[trip_u];
            curr_solution->loads[trip_v] = next_solution->loads[trip_v];
            if (flag) {
                add_element(curr_solution->Route[trip_v], inv_x, pos_v);
                add_element(curr_solution->Route[trip_v], u, pos_v);
            } else {
                add_element(curr_solution->Route[trip_v], u, pos_v + 1);
                add_element(curr_solution->Route[trip_v], inv_x, pos_v + 2);
            }

            delete_element(curr_solution->Route[trip_u], pos_u+1);
            delete_element(curr_solution->Route[trip_u], pos_u);
            return 1;
        }
    }

    if (inv_x > 0 && inv_u > 0)
    {
        if (pos_v == 2)
        {
            flag = 1;
            next_solution->total_cost = curr_solution->total_cost
                                        - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                        - min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                        - min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[inv_u].head_node]
                                        + min_cost[inst_tasks[inv_u].tail_node][inst_tasks[inv_x].head_node]
                                        + min_cost[inst_tasks[inv_x].tail_node][inst_tasks[v].head_node];
        } else {
            flag = 0;
            next_solution->total_cost = curr_solution->total_cost
                                        - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                        - min_cost[inst_tasks[u].tail_node][inst_tasks[x].head_node]
                                        - min_cost[inst_tasks[x].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+2]].head_node]
                                        - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                        + min_cost[inst_tasks[v].tail_node][inst_tasks[inv_u].head_node]
                                        + min_cost[inst_tasks[inv_u].tail_node][inst_tasks[inv_x].head_node]
                                        + min_cost[inst_tasks[inv_x].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];
        }
        next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
        if (next_solution->fitness < curr_solution->fitness)
        {
            curr_solution->total_cost = next_solution->total_cost;
            curr_solution->total_vio_loads = next_solution->total_vio_loads;
            curr_solution->fitness = next_solution->fitness;
            curr_solution->loads[trip_u] = next_solution->loads[trip_u];
            curr_solution->loads[trip_v] = next_solution->loads[trip_v];
            if (flag) {
                add_element(curr_solution->Route[trip_v], inv_x, pos_v);
                add_element(curr_solution->Route[trip_v], inv_u, pos_v);
            } else {
                add_element(curr_solution->Route[trip_v], inv_u, pos_v + 1);
                add_element(curr_solution->Route[trip_v], inv_x, pos_v + 2);
            }

            delete_element(curr_solution->Route[trip_u], pos_u+1);
            delete_element(curr_solution->Route[trip_u], pos_u);
            return 1;
        }
    }

    next_solution->total_cost = curr_solution->total_cost;
    next_solution->total_vio_loads = curr_solution->total_vio_loads;
    next_solution->fitness = curr_solution->fitness;
    next_solution->loads[trip_u] = curr_solution->loads[trip_u];
    next_solution->loads[trip_v] = curr_solution->loads[trip_v];
    return 0;
}

int lma_swap(lns_route *curr_solution, lns_route *next_solution, int u, int v, int trip_u, int trip_v, int pos_u, int pos_v, const Task *inst_tasks)
{

    next_solution->loads[trip_u] -= inst_tasks[u].demand - inst_tasks[v].demand;
    next_solution->loads[trip_v] += inst_tasks[u].demand - inst_tasks[v].demand;

    if (curr_solution->loads[trip_u] > capacity)
        next_solution->total_vio_loads -= curr_solution->loads[trip_u] - capacity;
    if (curr_solution->loads[trip_v] > capacity)
        next_solution->total_vio_loads -= curr_solution->loads[trip_v] - capacity;

    if (next_solution->loads[trip_u] > capacity)
        next_solution->total_vio_loads += next_solution->loads[trip_u] - capacity;
    if (next_solution->loads[trip_v] > capacity)
        next_solution->total_vio_loads += next_solution->loads[trip_v] - capacity;

    
    if (next_solution->total_vio_loads < 0)
    {
        printf("stop");
    }
    

    next_solution->total_cost = curr_solution->total_cost
                                - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[v].head_node]
                                + min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[u].head_node]
                                + min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];




    next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
    if (next_solution->fitness < curr_solution->fitness)
    {
        // check_cost1(*curr_solution, inst_tasks);
        curr_solution->total_cost = next_solution->total_cost;
        curr_solution->total_vio_loads = next_solution->total_vio_loads;
        curr_solution->fitness = next_solution->fitness;
        curr_solution->loads[trip_u] = next_solution->loads[trip_u];
        curr_solution->loads[trip_v] = next_solution->loads[trip_v];
        curr_solution->Route[trip_u][pos_u] = v;
        curr_solution->Route[trip_v][pos_v] = u;
        // check_cost1(*curr_solution, inst_tasks);
        if (curr_solution->loads[trip_u] > capacity || curr_solution->loads[trip_v] > capacity)
        {
            printf("stop");
        }
        return 1;
    }

    int inv_u = inst_tasks[u].inverse;
    if (inv_u > 0)
    {
        next_solution->total_cost = curr_solution->total_cost
                                    - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                    - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[v].head_node]
                                    + min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                    - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[inv_u].head_node]
                                    + min_cost[inst_tasks[inv_u].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];


        next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
        if (next_solution->fitness < curr_solution->fitness)
        {
            // check_cost1(*curr_solution, inst_tasks);
            curr_solution->total_cost = next_solution->total_cost;
            curr_solution->total_vio_loads = next_solution->total_vio_loads;
            curr_solution->fitness = next_solution->fitness;
            curr_solution->loads[trip_u] = next_solution->loads[trip_u];
            curr_solution->loads[trip_v] = next_solution->loads[trip_v];
            curr_solution->Route[trip_u][pos_u] = v;
            curr_solution->Route[trip_v][pos_v] = inv_u;
            // check_cost1(*curr_solution, inst_tasks);
            if (curr_solution->loads[trip_u] > capacity || curr_solution->loads[trip_v] > capacity)
            {
                printf("stop");
            }
            return 1;
        }
    }


    int inv_v = inst_tasks[v].inverse;
    if (inv_v > 0)
    {
        next_solution->total_cost = curr_solution->total_cost
                                    - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                    - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[inv_v].head_node]
                                    + min_cost[inst_tasks[inv_v].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                    - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[u].head_node]
                                    + min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];

        next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
        if (next_solution->fitness < curr_solution->fitness)
        {
            // check_cost1(*curr_solution, inst_tasks);
            curr_solution->total_cost = next_solution->total_cost;
            curr_solution->total_vio_loads = next_solution->total_vio_loads;
            curr_solution->fitness = next_solution->fitness;
            curr_solution->loads[trip_u] = next_solution->loads[trip_u];
            curr_solution->loads[trip_v] = next_solution->loads[trip_v];
            curr_solution->Route[trip_u][pos_u] = inv_v;
            curr_solution->Route[trip_v][pos_v] = u;
            // check_cost1(*curr_solution, inst_tasks);
            if (curr_solution->loads[trip_u] > capacity || curr_solution->loads[trip_v] > capacity)
            {
                printf("stop");
            }
            return 1;
        }
    }

    if (inv_v > 0 && inv_u > 0)
    {
        next_solution->total_cost = curr_solution->total_cost
                                    - min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[u].head_node]
                                    - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_u][pos_u-1]].tail_node][inst_tasks[inv_v].head_node]
                                    + min_cost[inst_tasks[inv_v].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                    - min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[v].head_node]
                                    - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                    + min_cost[inst_tasks[curr_solution->Route[trip_v][pos_v-1]].tail_node][inst_tasks[inv_u].head_node]
                                    + min_cost[inst_tasks[inv_u].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node];

        next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;
        if (next_solution->fitness < curr_solution->fitness)
        {
            // check_cost1(*curr_solution, inst_tasks);
            curr_solution->total_cost = next_solution->total_cost;
            curr_solution->total_vio_loads = next_solution->total_vio_loads;
            curr_solution->fitness = next_solution->fitness;
            curr_solution->loads[trip_u] = next_solution->loads[trip_u];
            curr_solution->loads[trip_v] = next_solution->loads[trip_v];
            curr_solution->Route[trip_u][pos_u] = inv_v;
            curr_solution->Route[trip_v][pos_v] = inv_u;
            // check_cost1(*curr_solution, inst_tasks);
            if (curr_solution->loads[trip_u] > capacity || curr_solution->loads[trip_v] > capacity)
            {
                printf("stop");
            }
            return 1;
        }
    }

    next_solution->total_cost = curr_solution->total_cost;
    next_solution->total_vio_loads = curr_solution->total_vio_loads;
    next_solution->fitness = curr_solution->fitness;
    next_solution->loads[trip_u] = curr_solution->loads[trip_u];
    next_solution->loads[trip_v] = curr_solution->loads[trip_v];
    return 0;
}

int lma_cross(lns_route *curr_solution, lns_route *next_solution, int u, int v, int trip_u, int trip_v, int pos_u, int pos_v, const Task *inst_tasks)
{
    int load_seg1 = 0, load_seg2 = 0;
    for (int i = 2; i <= pos_u; i++)
    {
        load_seg1 += inst_tasks[curr_solution->Route[trip_u][i]].demand;
    }
    for (int i = 2; i <= pos_v; i++)
    {
        load_seg2 += inst_tasks[curr_solution->Route[trip_v][i]].demand;
    }

    // case 1  // case 2 is not suitable for the directed CARP
    next_solution->loads[trip_u] = load_seg1 + curr_solution->loads[trip_v] - load_seg2;
    next_solution->loads[trip_v] = load_seg2 + curr_solution->loads[trip_u] - load_seg1;

    if (curr_solution->loads[trip_u] > capacity)
        next_solution->total_vio_loads -= curr_solution->loads[trip_u] - capacity;
    if (curr_solution->loads[trip_v] > capacity)
        next_solution->total_vio_loads -= curr_solution->loads[trip_v] - capacity;

    if (next_solution->loads[trip_u] > capacity)
        next_solution->total_vio_loads += next_solution->loads[trip_u] - capacity;
    if (next_solution->loads[trip_v] > capacity)
        next_solution->total_vio_loads += next_solution->loads[trip_v] - capacity;

    next_solution->total_cost = curr_solution->total_cost
                                - min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node]
                                - min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                + min_cost[inst_tasks[u].tail_node][inst_tasks[curr_solution->Route[trip_v][pos_v+1]].head_node]
                                + min_cost[inst_tasks[v].tail_node][inst_tasks[curr_solution->Route[trip_u][pos_u+1]].head_node];

    next_solution->fitness = next_solution->total_cost + LAMBDA * next_solution->total_vio_loads;



    if (next_solution->fitness < curr_solution->fitness)
    {
//            printf("case1 \n");
        curr_solution->total_cost = next_solution->total_cost;
        curr_solution->total_vio_loads = next_solution->total_vio_loads;
        curr_solution->fitness = next_solution->fitness;
        curr_solution->loads[trip_u] = next_solution->loads[trip_u];
        curr_solution->loads[trip_v] = next_solution->loads[trip_v];

        int tmp_route[500];
        memcpy(tmp_route, curr_solution->Route[trip_u], sizeof(curr_solution->Route[trip_u]));
        curr_solution->Route[trip_u][0] = pos_u;
        for (int i = pos_v + 1; i <= curr_solution->Route[trip_v][0]; i++)
        {
            curr_solution->Route[trip_u][0] ++;
            curr_solution->Route[trip_u][curr_solution->Route[trip_u][0]] = curr_solution->Route[trip_v][i];
        }

        curr_solution->Route[trip_v][0] = pos_v;
        for (int i = pos_u + 1; i <= tmp_route[0]; i++)
        {
            curr_solution->Route[trip_v][0] ++;
            curr_solution->Route[trip_v][curr_solution->Route[trip_v][0]] = tmp_route[i];
        }
        // check_cost1(*curr_solution, inst_tasks);
        return 1;
    }
    next_solution->total_cost = curr_solution->total_cost;
    next_solution->total_vio_loads = curr_solution->total_vio_loads;
    next_solution->fitness = curr_solution->fitness;
    next_solution->loads[trip_u] = curr_solution->loads[trip_u];
    next_solution->loads[trip_v] = curr_solution->loads[trip_v];
    return 0;
}



