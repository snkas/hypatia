# Point-to-point topology

A point-to-point topology is aimed to provide a quick way to generate a network topology with point-to-point links connecting nodes.

It encompasses the following files:

* `model/core/topology.h` - Base topology class
* `model/core/ptop-topology.cc/h` -- Point-to-point topology class


## Getting started

1. Add the following to the `config_ns3.properties` in your run folder:

   ```
   topology_ptop_filename="topology.properties"
   ```

2. Add the following topology file `topology.properties` to your run folder for a 3-ToR:

   ```
   num_nodes=4
   num_undirected_edges=3
   switches=set(0)
   switches_which_are_tors=set(0)
   servers=set(1,2,3)
   undirected_edges=set(0-1,0-2,0-3)
   
   link_channel_delay_ns=10000
   link_device_data_rate_megabit_per_s=100.0
   link_device_queue=drop_tail(100p)
   link_interface_traffic_control_qdisc=disabled
   ```

3. In your code, import the point-to-point topology:

   ```
   #include "ns3/ptop-topology.h"
   // Possibly also: 
   // #include "ns3/ipv4-arbiter-routing-helper.h"
   // #include "ns3/arbiter-ecmp-helper.h"
   ```

4. Before the start of the simulation run, in your code add to create the topology:

    ```c++
    // Read point-to-point topology
    Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
   
   // Install some type of routing, for example:
    ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
   
   // You can also use the traditional one, but it is very inefficient,
   // as it does linear look-up, which works poor with many sub-networks
   // which cannot be aggregated, as is the case for point-to-point topology. 
   
   // Ipv4GlobalRoutingHelper::PopulateRoutingTables();
   // You should have also given the Ipv4GlobalRoutingHelper() before.
    ```
   
5. From then on you can use it. For example to get the nodes:

    ```c++
    NodeContainer nodes = topology->GetNodes(); // Note: passed by reference
    ```


## Configuration properties

If one uses the default point-to-point topology, the following properties MUST also be defined in `config_ns3.properties`:
* `topology_ptop_filename` : Topology filename (relative to run folder)

## Topology file: topology.properties

Because the topology file can get quite big, and is independent to some extent, it is in a separate configuration file. This configuration file contains both the topological layout, and the link properties (delay, rate, queue size, qdisc).

**Example:**

```
num_nodes=4
num_undirected_edges=3
switches=set(0)
switches_which_are_tors=set(0)
servers=set(1,2,3)
undirected_edges=set(0-1,0-2,0-3)

link_channel_delay_ns=map(0-1: 10000, 0-2: 10000, 0-3: 10000)
link_device_data_rate_megabit_per_s=100.0
link_device_queue=map(0->1: drop_tail(50p), 1->0: drop_tail(100p), 0->2: drop_tail(100000B), 2->0: drop_tail(50p), 0->3: drop_tail(100p), 3->0: drop_tail(100p))
link_interface_traffic_control_qdisc=disabled
```

**Properties:**

* `num_nodes` (type: positive integer)

  Number of nodes.
  
* `num_undirected_edges` (type: positive integer)

  Number of undirected edges (each undirected edge will be expressed into two links).

* `switches` (type: set of node identifiers)

  All node identifiers which are switches expressed as `set(a, b, c)`, 
  e.g.: `set(5, 6)` means node 5 and 7 are switches.
  
* `switches_which_are_tors` (type: set of node identifiers)

  All node identifiers which are also ToRs expressed as `set(a, b, c)` 
  (type: set of node identifiers).
  
* `servers` (type: set of node identifiers)

  All node identifiers which are servers expressed as `set(a, b, c)`.
  
* `undirected_edges` (type: set of undirected edges)

  All undirected edges, expressed as `set(a-b, b-c)`, e.g.: `set(0-2, 2-3)` 
  means two undirected edges, between 0 and 2, and between 2 and 3.
  
* `all_nodes_are_endpoints` (optional) (type: boolean)

  Whether to allow all nodes to be endpoints for applications or not. 
  Normally, the topology only considers servers (or in absence thereof: ToRs) 
  as valid endpoints. Setting this to `true` will have the topology return 
  all nodes as valid endpoints. This is a soft enforce, meaning that only 
  code that checks with topology explicitly will adhere to the endpoints the 
  topology claims are permissible for applications.

* `link_channel_delay_ns` (type: undirected edge (two-link) mapping or a global value; a positive integer)

  Propagation delay set for undirected edges (=two-link) (ns).

* `link_device_data_rate_megabit_per_s` (type: directed edge (link) mapping or a global value; a positive double)

  Data rate set for links (Mbit/s). 
 
* `link_device_queue` (type: directed edge (link) mapping or a global value; `drop_tail(<integer>p)` or `drop_tail(<integer>B)`  for respectively packets or bytes)

  Queue implementation for link devices. Only DropTail is right now implemented.
  
* `link_interface_traffic_control_qdisc` (type: directed edge (link) mapping or a global value; string of `default`,  `disabled`, `fq_codel_better_rtt`)

   Interface traffic control queueing discipline for links.

All mandatory properties must be present. Empty sets are permitted. 
Empty lines and lines starting with a comment sign (#) are permitted. 
Besides it just defining a graph, the following rules apply:

* If there are servers defined, they can only have edges to a ToR.
* There is only a semantic difference between switches, switches which 
  are ToRs and servers. If there are servers, only servers should be 
  valid endpoints for applications. If there are no servers, ToRs should be valid endpoints instead.
