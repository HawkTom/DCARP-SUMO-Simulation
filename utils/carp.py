import numpy as np

class depot_out:
    edge = ""
    vt = False
    demand = 0
    
    @classmethod
    def set_depot(self, edgeID):
        depot_out.edge = edgeID
        

class depot_in:
    edge = ""
    vt = False
    demand = 0
    
    @classmethod
    def set_depot(self, edgeID):
        depot_in.edge = edgeID


class gc:
    sim = None
    depot = None
    min_cost = {}
    edge_map = {}
    capacity = 0
    max_vehicle = 0
    LAMBDA = np.inf
    vehicles = []
    veh_load = {}

    def __init__(self, len_edge):
        pass
        # gc.min_cost = -1 * np.ones((len_edge+1, len_edge+1))

    @classmethod
    def set_sim(self, sim):
        gc.sim = sim
    
    @classmethod
    def set_cap(self, capacity):
        gc.capacity = capacity
    
    @classmethod
    def set_cost(self,  i,  j, value):
        gc.min_cost["%d_to_%d"%(i,j)] = value

    @classmethod
    def set_edge_map(self, map_dict):
        gc.edge_map = map_dict
    
    @classmethod
    def set_max_vehicle(self, max_num):
        gc.max_vehicle = max_num
    
    @classmethod
    def add_vehicle(self, v):
        gc.vehicles.append(v)
    
    @classmethod
    def remove_vehicle(self, v):
        gc.vehicles.remove(v)
    
    @classmethod
    def set_veh_load(self, vid):
        gc.veh_load[vid] = 0
    @classmethod 
    def veh_add_load(self, vid, load):
        gc.veh_load[vid] += load



def cal_route_cost(sim, route):
    l = len(route)
    cost = 0
    for i in range(l-1):
        i1 = gc.edge_map[route[i].edge]
        i2 = gc.edge_map[route[i+1].edge]
        key_value = "%d_to_%d"%(i1,i2)
        if key_value not in gc.min_cost:
            rr = sim.findRoute(route[i].edge, route[i+1].edge, routingMode=1)
            tmp_cost = rr.travelTime
            gc.set_cost(i1, i2, tmp_cost)
        else:
            tmp_cost = gc.min_cost[key_value]
            
        cost += tmp_cost

    for i in range(1, l-1):
        i1 = gc.edge_map[route[i].edge]
        key_value = "%d_to_%d"%(i1,i1)
        if key_value not in gc.min_cost:
            rr = sim.findRoute(route[i].edge, route[i].edge, routingMode=1)
            tmp_cost = rr.travelTime
            gc.set_cost(i1, i1, tmp_cost)
        else:
            tmp_cost = gc.min_cost[key_value]
        cost -= tmp_cost
    return cost


def cal_edges_cost(sim, e1, e2):
    i1 = gc.edge_map[e1]
    i2 = gc.edge_map[e2]
    cost = gc.min_cost[i1, i2]
    if cost < 0:
        cost = sim.findRoute(e1, e2, routingMode=1).travelTime
    return cost


def cal_route_cost_with_vt1(sim, route):
    cost = 0
    l = len(route)
    current = route[0]
    for i in range(1, l-1):
        nxt = route[i]
        if nxt.vt:
            i1 = gc.edge_map[current.edge]
            i2 = gc.edge_map[depot_out.edge]
            key_value = "%d_to_%d"%(i1,i2)
            if key_value not in gc.min_cost:
                rr = sim.findRoute(current.edge, nxt.edge, routingMode=1)
                tmp_cost = rr.travelTime
                gc.set_cost(i1, i2, tmp_cost)
            else:
                tmp_cost = gc.min_cost[key_value]
            
            cost += tmp_cost

            key_value = "%d_to_%d"%(i2,i2)
            if key_value not in gc.min_cost:
                rr = sim.findRoute(depot_out.edge, depot_out.edge, routingMode=1)
                tmp_cost = rr.travelTime
                gc.set_cost(i2, i2, tmp_cost)
            else:
                tmp_cost = gc.min_cost[key_value]
            cost -= tmp_cost

            i3 = gc.edge_map[nxt.edge]
            key_value = "%d_to_%d"%(i3,i3)
            if key_value not in gc.min_cost:
                rr = sim.findRoute(nxt.edge, nxt.edge, routingMode=1)
                tmp_cost = rr.travelTime
                gc.set_cost(i3, i3, tmp_cost)
            else:
                tmp_cost = gc.min_cost[key_value]
            cost -= tmp_cost

        else:
            i1 = gc.edge_map[current.edge]
            i2 = gc.edge_map[nxt.edge]
            key_value = "%d_to_%d"%(i1,i2)
            if key_value not in gc.min_cost:
                rr = sim.findRoute(current.edge, nxt.edge, routingMode=1)
                tmp_cost = rr.travelTime
                gc.set_cost(i1, i2, tmp_cost)
            else:
                tmp_cost = gc.min_cost[key_value]
            
            cost += tmp_cost

            key_value = "%d_to_%d"%(i2,i2)
            if key_value not in gc.min_cost:
                rr = sim.findRoute(nxt.edge, nxt.edge, routingMode=1)
                tmp_cost = rr.travelTime
                gc.set_cost(i2, i2, tmp_cost)
            else:
                tmp_cost = gc.min_cost[key_value]
            cost -= tmp_cost

        current = route[i]
    
    i1 = gc.edge_map[current.edge]
    i2 = gc.edge_map[depot_in.edge]
    key_value = "%d_to_%d"%(i1,i2)
    if key_value not in gc.min_cost:
        rr = sim.findRoute(current.edge, depot_in.edge, routingMode=1)
        tmp_cost = rr.travelTime
        gc.set_cost(i1, i2, tmp_cost)
    else:
        tmp_cost = gc.min_cost[key_value]
    
    cost += tmp_cost


    return cost



def cal_route_cost_with_vt(route):
    cost = 0
    l = len(route)
    current = route[0]
    for i in range(1, l-1):
        nxt = route[i]

        cost += cal_edges_cost_with_vt(current, nxt)
        cost -= cal_edges_cost_with_vt(nxt, nxt)

        current = route[i]
    
    nxt = route[-1]
    cost += cal_edges_cost_with_vt(current, nxt)
    return cost

def cal_edges_cost_with_vt(e1, e2):
    if (not e1.vt) and e2.vt:
        i1 = gc.edge_map[e1.edge]
        i2 = gc.edge_map[depot_in.edge]
        key_value = "%d_to_%d"%(i1,i2)
        if key_value not in gc.min_cost:
            rr = gc.sim.findRoute(e1.edge, depot_in.edge, routingMode=1)
            cost = rr.travelTime
            gc.set_cost(i1, i2, cost)
        else:
            cost = gc.min_cost[key_value]
    
    if e1.vt and (not e2.vt):
        i1 = gc.edge_map[e1.edge]
        i2 = gc.edge_map[e2.edge]
        key_value = "%d_to_%d"%(i1,i2)
        if key_value not in gc.min_cost:
            rr = gc.sim.findRoute(e1.edge, e2.edge, routingMode=1)
            cost = rr.travelTime
            gc.set_cost(i1, i2, cost)
        else:
            cost = gc.min_cost[key_value]
        
        key_value = "%d_to_%d"%(i1,i1)
        if key_value not in gc.min_cost:
            rr = gc.sim.findRoute(e1.edge, e1.edge, routingMode=1)
            tmp_cost = rr.travelTime
            gc.set_cost(i1, i1, tmp_cost)
        else:
            tmp_cost = gc.min_cost[key_value]
        cost -= tmp_cost

    if (not e1.vt) and (not e2.vt):
        i1 = gc.edge_map[e1.edge]
        i2 = gc.edge_map[e2.edge]
        key_value = "%d_to_%d"%(i1,i2)
        if key_value not in gc.min_cost:
            rr = gc.sim.findRoute(e1.edge, e2.edge, routingMode=1)
            cost = rr.travelTime
            gc.set_cost(i1, i2, cost)
        else:
            cost = gc.min_cost[key_value]
        

    if e1.vt and e2.vt:
        if e1.edge == e2.edge:
            cost = 0
        else:
            i1 = gc.edge_map[e1.edge]
            i2 = gc.edge_map[depot_in.edge]
            key_value = "%d_to_%d"%(i1,i2)
            if key_value not in gc.min_cost:
                rr = gc.sim.findRoute(e1.edge, depot_in.edge, routingMode=1)
                cost = rr.travelTime
                gc.set_cost(i1, i2, cost)
            else:
                cost = gc.min_cost[key_value]
            
            key_value = "%d_to_%d"%(i1,i1)
            if key_value not in gc.min_cost:
                rr = gc.sim.findRoute(e1.edge, e1.edge, routingMode=1)
                tmp_cost = rr.travelTime
                gc.set_cost(i1, i1, tmp_cost)
            else:
                tmp_cost = gc.min_cost[key_value]
            cost -= tmp_cost
    return cost



class task():
    def __init__(self, edgeID, demand, vt=False):
        # self.id = index
        self.edge = edgeID
        self.demand = demand
        self.vt = vt


class routine():

    def __init__(self):
        # self.__len = 10
        self.tasks_seqs = [depot_out()]
        self.load = 0
        self.cost = 0
        self.vid = None
    
    def add_tasks(self, task):
        self.tasks_seqs.append(task)
        self.load += task.demand
    
    def complete(self):
        self.tasks_seqs.append(depot_in())
        self.cost = cal_route_cost_with_vt(self.tasks_seqs)


class solution():
    
    def __init__(self):
        self.routines = []
        # self.loads = []
        self.totalcost = 0
        self.total_vio_load = 0
        self.veh_route = {}
    
    def add_routine(self, routine):
        self.routines.append(routine)
        self.totalcost += routine.cost
        # self.loads.append(routine.load)
        self.total_vio_load += max(0, routine.load - gc.capacity)
    
    def complete(self):
        self.fitness = self.totalcost + gc.LAMBDA * self.total_vio_load
    
    def calculate_total_cost(self):
        self.totalcost = 0
        for r in self.routines:
            self.totalcost += r.cost


class dis_flag():
    def __init__(self):
        self.flags = {}

    def add_dis_flags(self, vid, dis):
        self.flags[vid] = dis
    
    def clear(self):
        self.flags = {}
