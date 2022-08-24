import time, copy, os
import sumolib, traci  # noqa
import pickle
import subprocess
import xml.etree.ElementTree as ET
from utils.carp import depot_in, depot_out, gc
from utils.carp import task, routine, solution, dis_flag
from utils.carp import cal_route_cost_with_vt
import numpy as np
from utils.plot_net_selection import plot_route

class sumo_dcarp_init():

    # pre-defined new tasks
    # TODO: put new tasks in the seperate scenario file
    new_tasks_whole = [3810,5933,1001,5831,5917,1830,3728,278,2788,4590,4729,942,3617,4041,2618,554,5545,2981,2721,354]
    busy_area = [6513,5609,2056,540,3935,2220,1199,3474,4086,3685,4340,4200,3389,5355,2901,3399,1606,5114,178,4063]
    not_busy_area = [5708,941,1411,6288,5831,6400,1832,5752,2618,6582,1326,5081,6521,5707,5343,4176,483,2786,498,1361]

    @classmethod
    def init(self, filepath):
        fs = sumo_dcarp_init.init_solution(filepath)
        sumo_dcarp_init.add_routes(fs)
        disflags = dis_flag()
        return fs, disflags

    @classmethod
    def init_solution(self, scenario):
        global edge_list
        # scenario = "dcarp/scenario1.xml"
        tree = ET.ElementTree(file=scenario)
        init_solution = tree.getroot().find("solution")

        x1 = list(init_solution.iter("task"))[0].attrib["seq"]
        x2 = list(init_solution.iter("demand"))[0].attrib["seq"]
        tasks_seqs = list(map(lambda x:int(x), x1.split()))
        demand_seqs = list(map(lambda x:int(x), x2.split()))

        fs = solution() # first solution
        route = routine()
        for t, d in zip(tasks_seqs[1:], demand_seqs[1:]):
            if t == 0:
                route.complete()
                fs.add_routine(route)
                del route
                route = routine() # new route
                continue
            
            tt = task(edge_list[t], d) # constrcut tasks
            route.add_tasks(tt)
            traci.gui.toggleSelection(tt.edge,  objType='edge')
        return fs


    @classmethod
    def add_routes(self, es):
        route_num = 0
        
        for k in range(len(es.routines)):
            route_edges_seqs = ()
            route = es.routines[k]
            task_seqs = route.tasks_seqs
            for i in range(len(task_seqs)-1):
                start = task_seqs[i].edge
                end = task_seqs[i+1].edge
                rr = traci.simulation.findRoute(start, end, routingMode=1)
                route_edges_seqs += rr.edges[:-1]

                rr.length

            route_edges_seqs += (depot_in.edge, )
            route_num += 1
            route_id = str(time.time()+route_num).replace('.', '')
            vechile_id = "nv" + str(route_num)
            traci.route.add(route_id, route_edges_seqs)
            # type = delivery (self defined in rou.xml)
            traci.vehicle.add(vechile_id, route_id, typeID="carp", depart=traci.simulation.getTime()+1) 
            traci.vehicle.setColor(vechile_id, (255,0,0))
            traci.vehicle.setParameter(vechile_id, "has.rerouting.device", "true")
            traci.vehicle.setParameter(vechile_id, "device.rerouting.period", 100000.0)
            gc.add_vehicle(vechile_id)
            es.veh_route[vechile_id] = k
            es.routines[k].vid = vechile_id
            gc.set_veh_load(vechile_id)
            traci.gui.toggleSelection(vechile_id)
            # break
        return route_num


class sumo_dcarp():

    def __init__(self):
        pass
    
    @classmethod
    def detect_vehicles(self):
        for vid in gc.vehicles:
            try:
                traci.vehicle.getLaneID(vid)
            except:
                gc.remove_vehicle(vid)
    
    @classmethod
    def reschedule(self, scenario, instance):
        global disflags, fs
        # determine which tasks have been served first.
        served_tasks = {}
        for vid in disflags.flags.keys():
            if vid not in gc.vehicles:
                served_tasks[vid] = -1
            else:
                flags = disflags.flags[vid]
                task_num = len(flags)
                dis = traci.vehicle.getDrivingDistance(vid, depot_in.edge, 1)
                # dis >= flags[j]: not been served; dis < flags[j]: been served
                for j in range(1, task_num+1):
                    if dis >= flags[j-1]:
                        break
                served_tasks[vid] = j
        
        # construct virtual tasks 
        for vid in disflags.flags.keys():
            if served_tasks[vid] < 0:
                continue

            # 1. obtain stop locations

            edge_id = traci.vehicle.getRoadID(vid)
            stop_edge = net.getEdge(edge_id).getID()
            

            # 2. obtaining served load
            i = fs.veh_route[vid]
            for j in range(1, served_tasks[vid]):
                remove_tasks = fs.routines[i].tasks_seqs.pop(1)
                if gc.veh_load[vid] == 1:
                    gc.veh_load[vid] -= 1
                gc.veh_add_load(vid, remove_tasks.demand)
            
            # make virtual tasks' demand not equal to 0
            if  gc.veh_load[vid] == 0:
                gc.veh_load[vid] += 1
            # construct new solutions
            vt = task(stop_edge, gc.veh_load[vid], vt=True)
            fs.routines[i].tasks_seqs.insert(1, vt)


            fs.routines[i].cost = cal_route_cost_with_vt(fs.routines[i].tasks_seqs)

        remove_idx = []
        for vid in disflags.flags.keys():
            if served_tasks[vid] < 0:
                remove_idx.append(fs.veh_route[vid])
        remove_idx.sort(reverse=True)
        
        for idx in remove_idx:
            fs.routines.pop(idx)

        fs.veh_route = {}
        for i in range(len(fs.routines)):
            fs.veh_route[fs.routines[i].vid] = i
            
        fs.complete()
        
        # if has new tasks, assign to the new vehicle (path-scanning)
        
        # fs --> new solutions with virtual tasks
        if scenario in [1,2,7,9]:
            new_tasks = sumo_dcarp_init.new_tasks_whole
        elif scenario in [3,6,8,12]:
            new_tasks = sumo_dcarp_init.busy_area
        elif scenario in [4,5,10,11]:
            new_tasks = sumo_dcarp_init.not_busy_area


        added_tasks = []
        if len(new_tasks) > 0 and instance > 3:
            print("added tasks current time:", traci.simulation.getTime())
            for i in range(4):
                added_tasks.append(new_tasks.pop())
        


        folder = "xml/scenario{0}_instance{1}".format(scenario, instance)
        solution_path = folder + "/new_solution.xml"
        

        if os.path.isfile(solution_path):
            # load new solution and assign to the vehicle
            sumo_dcarp.load_new_schedule(solution_path)
        else:
            if not os.path.exists(folder):
                # calculate and save the distance matrix for the map
                sumo_dcarp.duaroute_cal_route_cost(folder, added_tasks)

                # save the current solution for local search
                remain_tasks_num = sumo_dcarp.parse_tasks_seq(scenario, instance, added_tasks)

            if remain_tasks_num > 3:
                # call C program (local search)
                subprocess.call([r'./optimizer', str(scenario), str(instance)])

                # load new solution and assign to the vehicle
                sumo_dcarp.load_new_schedule(solution_path)
            else:
                for i in range(len(fs.routines)):
                    if (fs.routines[i].tasks_seqs[1].vt):
                        fs.routines[i].tasks_seqs.pop(1)



    

    @classmethod
    def duaroute_cal_route_cost(self, folder, added_tasks):
        global edge_list,fs

        os.mkdir(folder)

        for v in gc.vehicles:
            try:
                traci.vehicle.getColor(v)
            except:
                gc.vehicles.remove(v)
        

        root = ET.Element('meandata')
        for eid in edge_list[1:]:
            t = []
            for v in gc.vehicles:
                t.append(float(traci.vehicle.getParameter(v, "device.rerouting.edge:"+eid)))
            ET.SubElement(root,'edge', {'id':eid, 'traveltime':str(np.mean(t))})
        tree=ET.ElementTree(root)
        tree.write(folder+"/weights.xml")


        tasks = [depot_out.edge, depot_in.edge]
        for r in fs.routines:
            for tsk in r.tasks_seqs[1:-1]:
                tasks.append(tsk.edge)
        
        for tsk in added_tasks:
            tsk_eid = edge_list[tsk]
            traci.gui.toggleSelection(tsk_eid,  objType='edge')
            tasks.append(tsk_eid)
        
        root = ET.Element('routes')
        curr_time = str(traci.simulation.getTime())
        k = 0
        for src in tasks:
            k += 1
            dst = src
            ET.SubElement(root,'trip', {'id':str(k), 'depart': curr_time, 'from':src, 'to':dst})
        
        for src in tasks:
            for dst in tasks: 
                if src == dst:
                    continue
                k += 1
                ET.SubElement(root,'trip', {'id':str(k), 'depart': curr_time, 'from':src, 'to':dst})
        tree=ET.ElementTree(root)
        tree.write(folder+"/trips.xml")
        
        subprocess.call(["duarouter","--route-files", folder+"/trips.xml","--net-file","scenario/DCC.net.xml","--weight-files",folder+"/weights.xml","--bulk-routing","true", "--output-file",folder+"/result.rou.xml"])

    @classmethod
    def parse_tasks_seq(self, scenario, instance, added_tasks):
        global fs,net
        root = ET.Element('info', {'scenario': str(scenario), 'instance':str(instance)})
        
        
        depot_from_node = net.getEdge(depot_out.edge).getFromNode().getID()
        depot_to_node = net.getEdge(depot_out.edge).getToNode().getID()
        ET.SubElement(root,'depot', {'id':'0', 'edge': depot_out.edge, 'from_node': depot_from_node, 'from_index':str(node_dict[depot_from_node]), 'to_node':depot_to_node, 'to_index':str(node_dict[depot_to_node])}) # depot_out: 0
        ET.SubElement(root,'depot', {'id':'1', 'edge': depot_in.edge, 'from_node': depot_to_node, 'from_index':str(node_dict[depot_to_node]), 'to_node':depot_from_node,'to_index':str(node_dict[depot_from_node])}) # depot_in: 1

        ET.SubElement(root, 'capacity', {'value':str(gc.capacity)})

        k = 0
        remain_tasks_num = 0
        sol = ET.SubElement(root, 'solution')
        for vid in fs.veh_route:
            if vid not in gc.vehicles:
                continue
            r = fs.routines[fs.veh_route[vid]]
            # if all tasks in the route have been served and the remaining edges smaller than 10 edges. ignore this route. 
            route_edges = traci.vehicle.getRoute(vid)
            curr_idx = traci.vehicle.getRouteIndex(vid)
            if (len(route_edges) - curr_idx < 10) and len(r.tasks_seqs) == 3:
                continue
            
            rt = ET.SubElement(sol, 'route')
            for tsk in r.tasks_seqs[1:-1]:
                k += 1
                from_node = net.getEdge(tsk.edge).getFromNode().getID()
                to_node = net.getEdge(tsk.edge).getToNode().getID()
                if tsk.vt:
                    vt = vid[2:]
                else:
                    vt = '0'
                    remain_tasks_num += 1
                ET.SubElement(rt,'task', {'id':str(k), 'edge': tsk.edge, 'from_node': from_node, 'from_index':str(node_dict[from_node]), 'to_node':to_node,'to_index': str(node_dict[to_node]), 'demand':str(tsk.demand), 'id_index':str(edge_dict[tsk.edge]), 'vt':vt})

        if (len(added_tasks) > 0):
            rt = ET.SubElement(sol, 'route', {"property":"new"})
            for tsk in added_tasks:
                k += 1
                tsk_id = edge_list[tsk]
                from_node = net.getEdge(tsk_id).getFromNode().getID()
                to_node = net.getEdge(tsk_id).getToNode().getID()
                tsk_demand = int(net.getEdge(tsk_id).getLength() / 10)
                vt = '0'
                remain_tasks_num += 1
                ET.SubElement(rt,'task', {'id':str(k), 'edge': tsk_id, 'from_node': from_node, 'from_index':str(node_dict[from_node]), 'to_node':to_node,'to_index': str(node_dict[to_node]), 'demand':str(tsk_demand), 'id_index':str(edge_dict[tsk_id]), 'vt':vt})

        tree=ET.ElementTree(root)
        folder = "xml/scenario{0}_instance{1}".format(scenario, instance)
        tree.write(folder+"/solution.xml")
        return remain_tasks_num

    @classmethod
    def load_new_schedule(self, solution_path):
        global fs
        new_sol = solution()
        
        tree = ET.ElementTree(file=solution_path)
        root = tree.getroot()
        for route in root:
            new_route = routine()

            route_tasks = route.getchildren()

            vtsk = route_tasks.pop(0)
            e = edge_list[int(vtsk.attrib["IDindex"])]
            vt = int(vtsk.attrib["vt"])

            route_edges = []
            if (vt > 0):
                new_route.tasks_seqs.pop(0)
                vid = "nv"+str(vt)
            
            if (vt == 0): # assign a new vehicle
                vid = "nv" + str(int(gc.vehicles[-1][2:])+1)
                route_edges.append(depot_out.edge)
                gc.add_vehicle(vid)
                gc.set_veh_load(vid)
                demand = int(vtsk.attrib["demand"])
                new_route.add_tasks(task(e, demand))
                
            route_edges.append(e)

            for tsk in route_tasks:
                e = edge_list[int(tsk.attrib["IDindex"])]
                demand = int(tsk.attrib["demand"])
                new_route.add_tasks(task(e, demand))
                route_edges.append(e)
            
            route_edges.append(depot_in.edge)
            route_edges_paths = []
            for i in range(len(route_edges)-1):
                start = route_edges[i]
                end = route_edges[i+1]
                rr = traci.simulation.findRoute(start, end, routingMode=1)
                route_edges_paths += rr.edges[:-1]
            route_edges_paths.append(depot_in.edge)
            

            if (vt == 0):
                route_id = "new" + str(time.time())
                traci.route.add(route_id, route_edges_paths)
                traci.vehicle.add(vid, route_id, typeID="carp", depart=traci.simulation.getTime()+1) 
                traci.vehicle.setColor(vid, (255,0,0))
                traci.vehicle.setParameter(vid, "has.rerouting.device", "true")
                traci.vehicle.setParameter(vid, "device.rerouting.period", 100000.0)
                traci.gui.toggleSelection(vid)
            
            traci.vehicle.setRoute(vid, route_edges_paths)
            new_route.complete()
            new_route.tasks_seqs.insert(0, depot_out)
            new_route.vid = vid
            new_sol.add_routine(new_route)
            new_sol.veh_route[vid] = len(new_sol.routines)-1

        
        for vid in new_sol.veh_route:
            if vid in fs.veh_route:
                fs.routines[fs.veh_route[vid]] = copy.deepcopy(new_sol.routines[new_sol.veh_route[vid]])
            else:
                fs.add_routine(new_sol.routines[new_sol.veh_route[vid]])
                fs.veh_route[vid] = len(fs.routines) - 1


# remove vehicles which have returned to the depot
def remove_return_vehicle():
    veh_num = len(gc.vehicles)
    for i in range(veh_num):
        vid = gc.vehicles[i]
        try:
            traci.vehicle.getLength(vid)
        except:
            gc.vehicles[i] = None
    gc.vehicles = list(filter(lambda vid: vid != None, gc.vehicles))

def is_all_not_in_juction():
    for v in gc.vehicles:
        lid = traci.vehicle.getLaneID(v)
        if (len(lid) == 0):
            return False
        if lid[0] == ":":
            return False
    return True

def is_all_start():
    for v in gc.vehicles:
        l = traci.vehicle.getDistance(v)
        if l < 0:
            return False
    return True
        
'''
getDistance Returns the distance to the starting point like an odometer.
getDrivingDistance Return the distance to the given edge and position along the vehicles route.
getDrivingDistance: To the start of the edge
'''
def set_task_dis_flag1():
    global fs, disflags
    disflags.clear()
    for vid in gc.vehicles:
        dd = traci.vehicle.getDrivingDistance(vid, depot_in.edge, 1)

        idx = fs.veh_route[vid]
        task_seqs = fs.routines[idx].tasks_seqs
        distance_for_task = []
        for i in range(1, len(task_seqs) - 1):
            # calculate the distance of each task to the incoming depot to help calculate tasks which have been served
            dd1 = dd - traci.vehicle.getDrivingDistance(vid, task_seqs[i].edge, 0)
            distance_for_task.append(dd1)
        distance_for_task.append(0)
        disflags.add_dis_flags(vid, distance_for_task)



class DCARPListener(traci.StepListener):
    
    def __init__(self, scenario):
        self.period = 1000
        self.detect_freq = 30
        self.count = 0
        self.flag1 = True
        self.flag2 = False
        self.cost = [0]*5
        self.fo = open("output/cost_change"+str(scenario)+".txt", "w")
        self.net_fig = None
        self.net_ax = None
        self.instance = 0
        self.scenario = scenario

    def step(self, t=0):
        self.count += 1
        remove_return_vehicle()
        self.hidden_served_task()
        # accumulate cost used

        if self.instance >= 10:
            return True

        if self.flag1:
            flag2 = is_all_start()
            if flag2:
                # calculate the distanc and distance flag
                set_task_dis_flag1()
                self.flag1 = False
            else:
                return True
        
        if self.count % self.detect_freq == 0 or self.flag2:
            cost = self.detect_cost_change()

            if self.flag2 == False:
                t_now = traci.simulation.getTime()
                self.fo.write("{0},{1}\n".format(t_now, cost))


            self.cost.pop(0)
            self.cost.append(cost)            
            if self.cost[0]!=0 and self.cost[-1] > self.cost[0] * 1.05 or self.flag2:
                if not is_all_not_in_juction():
                    self.flag2 = True
                    return True
            
                cost = self.detect_cost_change()
                t_now = traci.simulation.getTime()
                self.fo.write("{0},{1}\n".format(t_now, cost))

                self.instance += 1
                print("scenario: ", self.scenario, "instance: ", self.instance, "current time:", traci.simulation.getTime())
                sumo_dcarp.reschedule(self.scenario, self.instance)

                # for vid in gc.vehicles:
                #     route_edges_nv1 = traci.vehicle.getRoute(vid)
                #     self.net_fig, self.net_ax = route_visualization(route_edges_nv1, fig=self.net_fig, ax=self.net_ax)
                #     self.net_fig.savefig("output/img/{0}_{1}.png".format(vid, self.instance), dpi=600)

                cost = self.detect_cost_change()
                t_now = traci.simulation.getTime()
                self.fo.write("optimise,{0},{1}\n".format(t_now, cost))

                self.flag1 = True
                self.flag2 = False

        return True

    def detect_cost_change(self):
        # calculate cost and draw in the window
        remove_return_vehicle()
        cost_future = 0
        for vid in gc.vehicles:
            route_edges = traci.vehicle.getRoute(vid)
            curr_idx = traci.vehicle.getRouteIndex(vid)
            vcost = 0
            for eid in route_edges[curr_idx+1:]:
                vcost += float(traci.vehicle.getParameter(vid, "device.rerouting.edge:"+eid))
            cost_future += vcost 
            # print(vid, vcost)
        # print("cost_future", cost_future)
        return cost_future
    
    def vis_route_in_gui(self):
        route_edges_nv1 = traci.vehicle.getRoute('nv1')
        for e in route_edges_nv1:
            traci.gui.toggleSelection(e,  objType='edge')
    def hidden_served_task(self):
        for vid in gc.vehicles:
            e = traci.vehicle.getRoadID(vid)
            if e == '':
                continue
            if traci.gui.isSelected(e,  objType='edge'):
                traci.gui.toggleSelection(e,  objType='edge')


class SCARPListener(traci.StepListener):
    
    def __init__(self, scn_idx) -> None:
        self.count = 0
        self.scenario = scn_idx
        self.curr_veh = 0
        self.curr_veh_load = 1000
        self.curr_veh_dis = []
        self.tasks_seqs = []
        # self.added_tasks_time_points = [34034.0, 34320.0, 34352.0, 34381.0]
        # self.added_tasks_time_points = [50000.0]
        # self.added_tasks_time_points = [33601.0, 34349.0, 34562.0, 34680.0, 34839.0]
        # self.added_tasks_time_points = [34321.0, 34350.0, 34470.0, 34500.0, 34530.0]
        # self.added_tasks_time_points = [50000.0]
        # self.added_tasks_time_points = [34413.0, 35311.0, 35340.0, 35379.0, 35400.0]
        # self.added_tasks_time_points = [44343.0, 44401.0, 44654.0, 45031.0, 45756.0]
        # self.added_tasks_time_points = [44737.0, 44882.0, 44913.0, 45150.0, 45189.0]
        # self.added_tasks_time_points = [44833.0, 44854.0, 44889.0, 44913.0, 45810.0]
        # self.added_tasks_time_points = [45150.0]
        # self.added_tasks_time_points = [50000.0]
        self.added_tasks_time_points = [45002.0,45480.0,45510.0,45540.0,45570.0]
        


    def step2(self, t=0): # added tasks one by one
        global edge_list
        self.count += 1
        self.hidden_served_task()
        # print(self.count)
        if self.count <= 200:
            return True
        vid = "new" + str(self.curr_veh)
        if self.curr_veh > 0 and (vid in gc.vehicles):
            if traci.vehicle.getDistance(vid) < 0:
                return True
            edge_id = traci.vehicle.getRoadID(vid)
            if edge_id[0] == ":":
                return True
        
        self.count = 0
        if self.scenario in [1,2,7,9]:
            new_tasks = sumo_dcarp_init.new_tasks_whole
        elif self.scenario in [3,6,8,12]:
            new_tasks = sumo_dcarp_init.busy_area
        elif self.scenario in [4,5,10,11]:
            new_tasks = sumo_dcarp_init.not_busy_area
        
        if len(new_tasks) > 0:
            added_task = new_tasks.pop()
        else:
            return True

        tsk_id = edge_list[added_task]
        from_node = net.getEdge(tsk_id).getFromNode().getID()
        to_node = net.getEdge(tsk_id).getToNode().getID()
        tsk_demand = int(net.getEdge(tsk_id).getLength() / 10)
        traci.gui.toggleSelection(tsk_id,  objType='edge')
        
        if ((vid not in gc.vehicles) or (self.curr_veh_load + tsk_demand > gc.capacity)):
            self.curr_veh += 1
            task_seqs = [depot_out.edge, tsk_id, depot_in.edge]

            self.tasks_seqs = [tsk_id]

            route_edges_seqs = ()
            for i in range(len(task_seqs)-1):
                start = task_seqs[i]
                end = task_seqs[i+1]
                rr = traci.simulation.findRoute(start, end, routingMode=1)
                route_edges_seqs += rr.edges[:-1]
                
            route_edges_seqs += (depot_in.edge, )
            route_num = self.curr_veh
            route_id = str(time.time()+route_num).replace('.', '')
            vechile_id = "new" + str(route_num)
            traci.route.add(route_id, route_edges_seqs)
            traci.vehicle.add(vechile_id, route_id, typeID="carp", depart=traci.simulation.getTime()+1) 
            traci.vehicle.setColor(vechile_id, (255,0,0))
            traci.vehicle.setParameter(vechile_id, "has.rerouting.device", "true")
            traci.vehicle.setParameter(vechile_id, "device.rerouting.period", 100000.0)
            traci.gui.toggleSelection(vechile_id)
            self.curr_veh_load = tsk_demand
            gc.vehicles.append(vechile_id)

            dis1 = traci.vehicle.getDrivingDistance(vechile_id, tsk_id, 0)
            dis0 = traci.vehicle.getDrivingDistance(vechile_id, depot_in.edge, 1)
            self.curr_veh_dis = [dis0-dis1]
        else:
            self.curr_veh_load += tsk_demand
            self.tasks_seqs.append(tsk_id)
            vid = "new" + str(self.curr_veh)

            
            dis = traci.vehicle.getDrivingDistance(vid, depot_in.edge, 1)
            for j in range(len(self.curr_veh_dis)):
                if dis >= self.curr_veh_dis[j]:
                    break
            remain_tasks = self.tasks_seqs[j:]

            edge_id = traci.vehicle.getRoadID(vid)
            stop_edge = net.getEdge(edge_id).getID()

            task_seqs = [stop_edge] + remain_tasks + [depot_in.edge]

            route_edges_seqs = ()
            for i in range(len(task_seqs)-1):
                start = task_seqs[i]
                end = task_seqs[i+1]
                rr = traci.simulation.findRoute(start, end, routingMode=1)
                route_edges_seqs += rr.edges[:-1]

            route_edges_seqs += (depot_in.edge, )
            traci.vehicle.setRoute(vid, route_edges_seqs)
            
            self.curr_veh_dis = []
            for tsk in self.tasks_seqs:
                dis1 = traci.vehicle.getDrivingDistance(vid, tsk, 0)
                dis0 = traci.vehicle.getDrivingDistance(vid, depot_in.edge, 1)
                self.curr_veh_dis.append(dis0-dis1)

        return True
    
    def step(self, t=0): # added tasks batch by batch, using path-scanning
        global edge_list
        # self.count += 1
        self.hidden_served_task()
        # print(self.count)
        if len(self.added_tasks_time_points) == 0:
            return True

        curr_time = traci.simulation.getTime()
        if curr_time != self.added_tasks_time_points[0]:
            return True
        self.added_tasks_time_points.pop(0)

        # if self.count <= 500:
        #     return True
        
        # self.count = 0

        if self.scenario in [1,2,7,9]:
            new_tasks = sumo_dcarp_init.new_tasks_whole
        elif self.scenario in [3,6,8,12]:
            new_tasks = sumo_dcarp_init.busy_area
        elif self.scenario in [4,5,10,11]:
            new_tasks = sumo_dcarp_init.not_busy_area
        
        added_tasks = []
        added_tasks_demand = []
        if len(new_tasks) > 0:
            print("added tasks current time:", traci.simulation.getTime())
            for _ in range(4):
                tsk_id = edge_list[new_tasks.pop()]
                added_tasks.append(tsk_id)
                added_tasks_demand.append(int(net.getEdge(tsk_id).getLength() / 10))
                traci.gui.toggleSelection(tsk_id,  objType='edge')
        else:
            return True

        self.tasks_seqs = [depot_out.edge]
        # path-scanning
        load = 0
        while len(added_tasks) > 0:
            ratios = []
            for jj in range(len(added_tasks)):
                if (load+added_tasks_demand[jj] > gc.capacity):
                    continue
                rr = traci.simulation.findRoute(self.tasks_seqs[-1], added_tasks[jj])
                dis = rr.cost
                ratio = dis/(gc.capacity-load-added_tasks_demand[jj])
                ratios.append(ratio)
            if len(ratios) == 0:
                load = 0
                self.tasks_seqs += [depot_in.edge, depot_out.edge]
            else:
                maxr = max(ratios)
                select = ratios.index(maxr)
                self.tasks_seqs.append(added_tasks.pop(select))
                load += added_tasks_demand.pop(select)
        self.tasks_seqs.append(depot_in.edge)

        self.curr_veh += 1
        route_edges_seqs = ()
        for i in range(len(self.tasks_seqs)-1):
            start = self.tasks_seqs[i]
            end = self.tasks_seqs[i+1]
            rr = traci.simulation.findRoute(start, end, routingMode=1)
            route_edges_seqs += rr.edges[:-1]

        route_edges_seqs += (depot_in.edge, )
        route_num = self.curr_veh
        route_id = str(time.time()+route_num).replace('.', '')
        vechile_id = "new" + str(route_num)
        traci.route.add(route_id, route_edges_seqs)
        traci.vehicle.add(vechile_id, route_id, typeID="carp", depart=traci.simulation.getTime()+1) 
        traci.vehicle.setColor(vechile_id, (255,0,0))
        traci.vehicle.setParameter(vechile_id, "has.rerouting.device", "true")
        traci.vehicle.setParameter(vechile_id, "device.rerouting.period", 100000.0)
        traci.gui.toggleSelection(vechile_id)
        gc.vehicles.append(vechile_id)
        
        return True

        # from_node = net.getEdge(tsk_id).getFromNode().getID()
        # to_node = net.getEdge(tsk_id).getToNode().getID()

    def hidden_served_task(self):
        for vid in gc.vehicles:
            try:
                e = traci.vehicle.getRoadID(vid)
                if e == '':
                    continue
                if traci.gui.isSelected(e,  objType='edge'):
                    traci.gui.toggleSelection(e,  objType='edge')
            except:
                pass



png_index = -1
def route_visualization(route_edges, fig=None, ax=None):
    global png_index
    png_index += 1
    args = ["-n", "scenario/DCC.net.xml", "--xlim", "-100,7200", "--ylim", "-100,5400", "--xticks", "-100,7201,2000,16", "--yticks", "-100,5401,1000,16", "--selected-width", "2", "--edge-width", ".5", "--edge-color", "#606060", "--selected-color", "#800000", "--online", "on"] #"-o", "output/route_for_vid1_"+str(png_index)+".png",
    n_fig, n_ax = plot_route(route_edges, args, fig, ax)
    return n_fig, n_ax


# main program
with open('traffic/dublin.pickle', 'rb') as f:
    map_data = pickle.load(f)

node_dict = map_data[0]
node_list = map_data[1]
edge_dict = map_data[2]
edge_list = map_data[3]

# if (len(sys.argv) == 1):
#     raise ValueError("please input scenario index")
# scenario = int(sys.argv[1])

scenario = 1
scenario_file = "dcarp/scenario{0}.xml".format(scenario)
tree = ET.ElementTree(file=scenario_file)
info = tree.getroot()
time_setting = info.find("time")
begin_time = time_setting.attrib["begin"]
end_time = time_setting.attrib["end"]
step_length = time_setting.attrib["step"]

depot_setting = info.findall("depot")
if (depot_setting[0].attrib["id"] == "incoming"):
    depot_in.set_depot(depot_setting[0].attrib["edge"])
    depot_out.set_depot(depot_setting[1].attrib["edge"])

if (depot_setting[0].attrib["id"] == "outgoing"):
    depot_in.set_depot(depot_setting[1].attrib["edge"])
    depot_out.set_depot(depot_setting[0].attrib["edge"])


cap_setting = info.findall("capacity")[0]
gc.set_cap(int(cap_setting.attrib["value"])+1)
gc.set_edge_map(edge_dict)

net = sumolib.net.readNet("scenario/DCC.net.xml")
depot_coord = net.getEdge(depot_out.edge).getFromNode().getCoord()
traci.start(["sumo-gui", "-c", "scenario/DCC_simulation.sumo.cfg", "--begin", begin_time, "--end", end_time, "--step-length", step_length, "--start", "true"]) #, "--start", "true"
traci.poi.add('depot', depot_coord[0],depot_coord[1], (1,0,0,0))
gc.set_sim(traci.simulation)


fs, disflags = sumo_dcarp_init.init(scenario_file)


# the above is the initial process

version = "dynamic"
listener = DCARPListener(scenario)
# version = "static"
# listener = SCARPListener(scenario)

traci.addStepListener(listener)


while len(gc.vehicles) > 0:
    remove_return_vehicle()
    traci.simulationStep()

# if version == "static":
#     with open("output/simulationStepStatic.txt", "a") as f:
#         s = "scenario{0} ".format(scenario)
#         s += time.strftime(r"%Y-%m-%d:%H:%M:%S", time.localtime()) 
#         s +=  ": " + str(begin_time) + " to " + str(traci.simulation.getTime())
#         s += "\n"
#         f.write(s)
    
traci.close()
if version == "dynamic":
    listener.fo.close()
