# Arbiter routing

In the normal ns-3 routing classes, they restrict you to particular routing table structures. Arbiter routing is a new kind of routing made such that you can basically implement any arbitrary routing strategy.

It encompasses the following files:

* **Arbiter:** `model/core/arbiter.c/h`

  Arbiter abstract super class. It basically requires that any of its subclasses implements a function `decide(source_node_id, target_node_id, packet, ip_header, is_socket_request_for_source_ip)` which decides what to do with a packet, meaning whether to drop, or if to forward, to which interface and to which gateway IP address (it returns a tuple of `bool failed, uint32_t out_if_idx, uint32_t gateway_ip_address`).
  
* **ArbiterPtop:** `model/core/arbiter-ptop.c/h`

   Extends the `Arbiter` class and transforms the decide function into one for point-to-point topologies specifically: `topology_decide(source_node_id, target_node_id, neighbor_node_ids, packet, ip_header, is_socket_request_for_source_ip,)`. This decide function returns the next hop's node id (as there is no need for gateways in point-to-point), or `-1` if to drop.
   
* **ArbiterEcmp:** `model/core/arbiter-ecmp.c/h`

  Extends the `ArbiterPtop` class, and does a routing decision based on calculating a 5-tuple hash and then picking the next hop out of a list of next-hop options towards each destination.
   
* **ArbiterEcmpHelper:** `model/arbiter-ecmp-helper.c/h`

  Helper to calculate the routing state for the `ArbiterEcmp` instances and installs that routing state on them.
   
* **Ipv4ArbiterRouting:** `model/core/ipv4-arbiter-routing.c/h`

  Routing instance (`Ipv4ArbiterRouting`) which for every decision calls upon its own `Arbiter` instance, which makes the decision for each packet
  
* **Ipv4ArbiterRoutingHelper:** `helper/core/ipv4-arbiter-routing-helper.c/h`

   IPv4 arbiter routing helper -- used to install `Ipv4ArbiterRouting` instances on nodes


## Getting started: using the helpers

1. In your code, import the helpers:

   ```
   #include "ns3/ipv4-arbiter-routing-helper.h"
   #include "ns3/arbiter-ecmp-helper.h"
   ```

2. Before the start of the simulation run, in your code add to install ECMP routing on your point-to-point topology:

    ```c++
    // Create point-to-point topology, and install routing arbiters
    Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation);
    ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
   ```


## Getting started: creating your own arbiter

If you want to implement your own arbiter, you should create a class which inherits 
from `Arbiter` if you have a novel (non point-to-point) topology. If you have a 
point-to-point topology, you can inherit from `ArbiterPtop`. Effectively, you 
only have to implement one function, and you are good to go. However, of course 
you need to calculate some routing state possibly. For an example of calculating 
routing state, take a look at the `ArbiterEcmpHelper`.
