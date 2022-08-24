#include <iostream>
#include <string.h>
#include <map>

#include "utils.h"
#include <math.h>


using namespace tinyxml2;
using namespace std;

int trav_cost[NODE_NUM+1][NODE_NUM+1];
int min_cost[NODE_NUM+1][NODE_NUM+1];
bool links[ARC_NUM+1][ARC_NUM+1];
Arc roads[7000];
Task inst_tasks[301];


int req_edge_num;
int task_num;
int capacity;
int DEPOT;

void construct_edge_node_map(map<string, Arc> m);
void reschedule();
void save_solution_xml(const Individual* new_solution, const Task *inst_tasks);
void insert_new_tasks(Individual *solution, int *added_tasks, const Task *inst_tasks);

int terminal_duration = 5;
int scenario_idx = 3;
int instance_idx = 2;
char path5[50];

int main(int argc, char *argv[])
{
    // init();
    struct tm *tm_now;
    time_t now;
    now = time(NULL);
    tm_now = localtime(&now);
    printf("start datetime: %d-%d-%d %d:%d:%d\n",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec) ;
    int seed = (int)rand() + tm_now->tm_year + tm_now->tm_mon + tm_now->tm_mday + tm_now->tm_hour + tm_now->tm_min + tm_now->tm_sec;
    seed = 222;
    srand(seed);
    printf("seed: %d\n", seed);


    scenario_idx = atoi(argv[1]);
    instance_idx = atoi(argv[2]);
    reschedule();
    return 0;
}

void reschedule()
{
    map<string, Arc> Arcs;
    const char* arc_id;


    char path0[50];
    sprintf(path0, "xml/scenario%d_instance%d/solution.xml", scenario_idx, instance_idx);
    XMLDocument doc;
    doc.LoadFile(path0);
    XMLElement* info = doc.FirstChildElement( "info" );

    // int scenario_idx = atoi(info->Attribute("scenario"));
    // int instance_idx = atoi(info->Attribute("instance"));
    printf("scenario: %d; instance: %d\n", scenario_idx, instance_idx);

    XMLElement* depot_out = info->FirstChildElement( "depot" );
    XMLElement* depot_in = depot_out->NextSiblingElement();

    inst_tasks[0].head_node = atoi(depot_out->Attribute("from_index"));
    inst_tasks[0].tail_node = inst_tasks[0].head_node;

    int depot_node = inst_tasks[0].head_node;

    

    Arc _arc;
    arc_id = depot_out->Attribute("edge");
    _arc.head_node = inst_tasks[0].head_node;
    _arc.tail_node = atoi(depot_out->Attribute("to_index"));
    Arcs[arc_id] = _arc;

    arc_id = depot_in->Attribute("edge");
    _arc.tail_node = inst_tasks[0].head_node;
    _arc.head_node = atoi(depot_out->Attribute("to_index"));
    Arcs[arc_id] = _arc;


    XMLElement* cap = doc.FirstChildElement( "info" )->FirstChildElement( "capacity" );
    capacity = atoi(cap->Attribute("value"));

    Individual solution;
    memset(solution.Sequence, 0, sizeof(solution.Sequence));
    memset(solution.Loads, 0, sizeof(solution.Loads));
    solution.Sequence[0] = 0;
    int load = 0;
    solution.Loads[0] = -1;

    int idx, head_node, tail_node, demand, IDindex, vt;
    XMLElement* slt = doc.FirstChildElement( "info" )->FirstChildElement( "solution" );
    XMLElement* curr_route = slt->FirstChildElement( "route" );

    int stop[101];
    int remain_capacity[101];
    memset(stop, 0, sizeof(stop));
    memset(remain_capacity, 0, sizeof(remain_capacity));

    req_edge_num = 0;
    int added_tasks[100];
    memset(added_tasks, 0, sizeof(added_tasks));
    while (true)
    {
        XMLElement* curr_task = curr_route->FirstChildElement( "task" );
        XMLElement* next_task;
        solution.Sequence[0] ++;
        solution.Sequence[solution.Sequence[0]] = 0;
        solution.Loads[0] ++;
        solution.Loads[solution.Loads[0]] = load;
        load = 0;
        while (true)
        {   
            arc_id = curr_task->Attribute("edge");
            idx = atoi(curr_task->Attribute("id"));
            head_node = atoi(curr_task->Attribute("from_index"));
            tail_node = atoi(curr_task->Attribute("to_index"));
            demand = atoi(curr_task->Attribute("demand"));
            IDindex = atoi(curr_task->Attribute("id_index"));
            vt = atoi(curr_task->Attribute("vt"));

            if (vt)
            {
                inst_tasks[idx].head_node = depot_node;
            } else {
                inst_tasks[idx].head_node = head_node;
            }

            if (vt)
            {
                stop[0] ++;
                stop[stop[0]] = tail_node;
                remain_capacity[0] ++;
                remain_capacity[remain_capacity[0]] = capacity - demand;
            }

            inst_tasks[idx].tail_node = tail_node;
            inst_tasks[idx].demand = demand;
            inst_tasks[idx].vt = vt;
            inst_tasks[idx].IDindex = IDindex;
            inst_tasks[idx].inverse = -1;
            inst_tasks[idx].serv_cost = 0; // ------------------------ 

            solution.Sequence[0] ++;
            solution.Sequence[solution.Sequence[0]] = idx;
            load += demand;

            req_edge_num ++;
            Arc _arc;

            _arc.head_node = head_node;
            _arc.tail_node = tail_node;
            
            Arcs[arc_id] = _arc;

            next_task = curr_task->NextSiblingElement();
            if (next_task == 0) break;
            curr_task = next_task;
        }

        XMLElement* next_route = curr_route->NextSiblingElement();
        if (next_route == 0) break;
        if (next_route->Attribute("property"))
        {
            curr_route = next_route;
            curr_task = curr_route->FirstChildElement( "task" );
            while (true)
            {   
                arc_id = curr_task->Attribute("edge");
                idx = atoi(curr_task->Attribute("id"));
                head_node = atoi(curr_task->Attribute("from_index"));
                tail_node = atoi(curr_task->Attribute("to_index"));
                demand = atoi(curr_task->Attribute("demand"));
                IDindex = atoi(curr_task->Attribute("id_index"));
                vt = atoi(curr_task->Attribute("vt"));


                inst_tasks[idx].tail_node = tail_node;
                inst_tasks[idx].head_node = head_node;
                inst_tasks[idx].demand = demand;
                inst_tasks[idx].vt = vt;
                inst_tasks[idx].IDindex = IDindex;
                inst_tasks[idx].inverse = -1;
                inst_tasks[idx].serv_cost = 0; // ------------------------ 

                added_tasks[0]++;
                added_tasks[added_tasks[0]] = idx;

                req_edge_num ++;
                Arc _arc;

                _arc.head_node = head_node;
                _arc.tail_node = tail_node;
                
                Arcs[arc_id] = _arc;

                next_task = curr_task->NextSiblingElement();
                if (next_task == 0) break;
                curr_task = next_task;
            }
            next_route = curr_route->NextSiblingElement();
        }
        if (next_route == 0) break;
        curr_route = next_route;
    }
    solution.Sequence[0] ++;
    solution.Sequence[solution.Sequence[0]] = 0;
    solution.Loads[0] ++;
    solution.Loads[solution.Loads[0]] = load;


    DEPOT = inst_tasks[0].head_node;
    task_num = req_edge_num;

    // generate_min_cost
    construct_edge_node_map(Arcs);

    // insert new tasks into solution
    printf("\n");
    if (added_tasks[0] > 0)
    {
        insert_new_tasks(&solution, added_tasks, inst_tasks);
    }

    int i, j, t;
    for (i = 1; i <= solution.Sequence[0]; i++)
    {
        t = solution.Sequence[i];
        if (t == 0) continue;
        inst_tasks[t].serv_cost = min_cost[inst_tasks[t].head_node][inst_tasks[t].tail_node];
    }
    solution.TotalCost = get_task_seq_total_cost(solution.Sequence, inst_tasks);
    solution.TotalVioLoad = 0;
    
    Individual init_solution1;
    Individual bestSolution1;
    Individual gbest_solution;

    gbest_solution.TotalCost = INF;



    copy_individual(&init_solution1, &solution);
    HyLS(&init_solution1, &bestSolution1, inst_tasks);
    if (gbest_solution.TotalCost > bestSolution1.TotalCost)
    {
        copy_individual(&gbest_solution, &bestSolution1);
    }
    
    save_solution_xml(&gbest_solution, inst_tasks);
    
}

void insert_new_tasks(Individual *solution, int *added_tasks, const Task *inst_tasks)
{
    int i, j, k;
    int task;
    int avail_route[101];


    int Positions[101];
    int Route[101][MAX_TASK_SEQ_LENGTH];
    find_ele_positions(Positions, solution->Sequence, 0);
    Route[0][0] = Positions[0] - 1;
    for (i = 1; i < Positions[0]; i++)
    {
        AssignSubArray(solution->Sequence, Positions[i], Positions[i+1], Route[i]); // Route[i]: 0 x x x x 0
    }
    
    for(i = 1; i <= added_tasks[0]; i++)
    {
        task = added_tasks[i];
        memset(avail_route, 0, sizeof(avail_route));
        for (j = 1; j <= solution->Loads[0]; j ++)
        {
            if (solution->Loads[j] + inst_tasks[task].demand < capacity)
            {
                avail_route[0] ++;
                avail_route[avail_route[0]] = j;
            }
        }
        if (avail_route[0] == 0)
        {
            Route[0][0] ++;
            Route[Route[0][0]][0] = 3; 
            Route[Route[0][0]][1] = 0; 
            Route[Route[0][0]][2] = task; 
            Route[Route[0][0]][3] = 0; 

            solution->Loads[0] ++;
            solution->Loads[solution->Loads[0]] = inst_tasks[task].demand;
            continue;
        }

        int cost = INF, tmp_cost, u, v;
        int insert_route, insert_position;
        for (j = 1; j <= avail_route[0]; j++)
        {
            for (k = 1; k < Route[avail_route[j]][0]; k ++)
            {
                u = inst_tasks[Route[avail_route[j]][k]].tail_node;
                v = inst_tasks[Route[avail_route[j]][k+1]].head_node;
                tmp_cost = min_cost[u][inst_tasks[task].head_node] + min_cost[inst_tasks[task].tail_node][v] - min_cost[u][v];
                if (tmp_cost < cost)
                {
                    cost = tmp_cost;
                    insert_route = avail_route[j];
                    insert_position = k;
                }
            } 
        }
        add_element(Route[insert_route], task, insert_position+1);
        solution->Loads[insert_route] += inst_tasks[task].demand;
    }
    
    memset(solution->Sequence, 0, sizeof(solution->Sequence));
    solution->Sequence[0] = 1;
    solution->Sequence[1] = 0;
    for (i = 1; i <= Route[0][0]; i++)
    {
        for (j = 2; j <= Route[i][0]; j++)
        {
            solution->Sequence[0] ++;
            solution->Sequence[solution->Sequence[0]] = Route[i][j];
        }
    }
    // solution->TotalCost = get_task_seq_total_cost(solution->Sequence, inst_tasks);
}


void save_solution_xml(const Individual* new_solution, const Task *inst_tasks)
{
    XMLDocument doc;
    XMLElement* root = doc.NewElement("solution");
    root->SetAttribute("num", new_solution->Loads[0]);
    doc.InsertFirstChild(root);
    
    XMLElement* route;
    XMLElement* task;
    int i, j, k;
    k = 0;
    for (i = 1; i < new_solution->Sequence[0]; i++)
    {   
        j = new_solution->Sequence[i];
        if ( j == 0)
        {
            k ++;
            route = root->InsertNewChildElement("route");
            route->SetAttribute("index", k);
            continue;
        }
        task = route->InsertNewChildElement("task");
        task->SetAttribute("IDindex", inst_tasks[j].IDindex);
        task->SetAttribute("vt", inst_tasks[j].vt);
        task->SetAttribute("demand", inst_tasks[j].demand);
    }
    char path[50];
    sprintf(path, "xml/scenario%d_instance%d/new_solution.xml", scenario_idx, instance_idx);
    doc.SaveFile(path);

}

void construct_edge_node_map(map<string, Arc> m)
{
        // map<string, float> routes;
    char path[50], path1[50];
    sprintf(path, "xml/scenario%d_instance%d/result.rou.alt.xml", scenario_idx, instance_idx);
    sprintf(path1, "xml/scenario%d_instance%d/trips.xml", scenario_idx, instance_idx);

    XMLDocument doc, doc1;
    doc.LoadFile(path);
    doc1.LoadFile(path1);

    const char* rid;
    const char* cost;
    const char* rid1;
    // const char* cost;
    XMLElement* curr = doc.FirstChildElement( "routes" )->FirstChildElement( "vehicle" );
    XMLElement* next;
    XMLElement* next1;

    XMLElement* curr1 = doc1.FirstChildElement( "routes" )->FirstChildElement( "trip" );
    const char* from_edge;
    const char* to_edge;
    map<string, Arc> ::iterator l_it, l_it_to;

    int head, tail, head_to, tail_to, tmp_cost;
    memset(min_cost, -1, sizeof(min_cost));
    for (int i = 0; i <= NODE_NUM; i++)
    {
        min_cost[i][i] = 0;
    }
    while (true)
    {
        
        rid = curr->Attribute( "id" );
        rid1 = curr1->Attribute("id");
        if (strcmp(rid, rid1) != 0)
        {
            printf("error xml\n");
            exit(-1);
        }
        cost = curr->FirstChildElement()->FirstChildElement()->Attribute("cost");
        from_edge = curr1->Attribute("from");
        to_edge = curr1->Attribute("to");
        l_it = m.find(from_edge);
        head = l_it->second.head_node;
        tail = l_it->second.tail_node;
        tmp_cost = round(atof(cost));
        if (strcmp(from_edge, to_edge) == 0)
        {
            /* code */
            min_cost[head][tail] = tmp_cost;
            // cout << head << "\t" << tail << "\t" << tmp_cost << endl;
            
        } else {
            /* code */
            l_it_to = m.find(to_edge);
            head_to = l_it_to->second.head_node;
            tail_to = l_it_to->second.tail_node;
            if (min_cost[head][head_to] < 0) min_cost[head][head_to] = tmp_cost - min_cost[head_to][tail_to];
            if (min_cost[head][tail_to] < 0) min_cost[head][tail_to] = tmp_cost;
            if (min_cost[tail][head_to] < 0) min_cost[tail][head_to] = tmp_cost - min_cost[head][tail] - min_cost[head_to][tail_to];
            if (min_cost[tail][tail_to] < 0) min_cost[tail][tail_to] = tmp_cost - min_cost[head][tail];
        }
        // routes[rid] = atof(cost);
        next = curr->NextSiblingElement();
        next1 = curr1->NextSiblingElement();
        if (next == 0 || next1 == 0) break;
        curr = next;
        curr1 = next1;
    } 
}