# Getting started

You can either immediately start with the tutorial below, or read more documentation:

* `basic_simulation_and_run_folder.md` -- Basic concepts of the framework
* `ptop_topology.md` -- Point-to-point topology
* `arbiter_routing.md` -- A new type of routing with more flexibility
* `link_utilization_tracking.md` -- Link utilization tracking
* `link_queue_tracking.md` -- Link queue tracking
* `flows_application.md` -- Flow application ("send from A to B a flow of size X at time T")
* `udp_burst_application.md` -- UDP burst application ("send from A to B at a rate of X Mbit/s at time T for duration D")
* `pingmesh_application.md` -- Ping application ("send from A to B a ping at an interval I")
* `tcp_optimizer.md` -- Optimize certain TCP parameters
* `future_work.md` -- To find out what can be extended / improved


## Tutorial


### Goal

We are going to have a single switch which has 3 servers under it. A switch which has servers directly under it is typically referred to as a ToR. Link delay is 10 microseconds, link rate is 100 Mb/s. Each link interface has a FIFO queue of 100 packets. The simulation will be run for 4 seconds. It looks visually as follows:

```
           Switch (id: 0)
         /     |          \
       /       |           \
     /         |            \
 Server (1)  Server (2)  Server (3)
```

We are going to install three different applications:

1. Three TCP flows:
   * F0) From server 1 to 3 send 100 Mbit starting at T=0
   * F1) From server 2 to 3 send 100 Mbit starting at T=500ms
   * F2) From server 2 to 3 send 100 Mbit starting at T=500ms
   
2. Two UDP bursts:
   * B0) From server 1 to 2 send at 50 Mbit/s starting at T=0 for 5s
   * B1) From server 2 to 1 send at 50 Mbit/s starting at T=0 for 5s
   
3. Send pings between all servers at an interval of 10ms


### Expectations

* F0) Should get 50 Mbit/s for the first 500ms. Afterwards, it will still get 50 Mbit/s approximately, as F1/F2 will have to compete on the 2-0 link. `100 Mbit / 50 Mbit/s = 2s`, which means it will finish in time.

* F1) Shares what B2 leaves with F2, as such get 20-30 Mbit/s its entire lifetime, which is 3.5s until end. `3.5s * 25 Mbit/s = 87.5 Mbit`, so it will not finish before the end.

* F2) The same as F1.

* B1) Most of its 50 Mbit/s should be received on the other side, as the TCP flows will back down whereas this one will not.

* B2) The same as B1.

* Especially the pings between 1 and 2 will be a bit longer as there is the queue build-up due to congestion.


### Doing the experiments

1. Create a directory anywhere called `example_run`, and create four files in there:

   **config_ns3.properties**
   
   ```
   simulation_end_time_ns=4000000000
   simulation_seed=123456789
    
   topology_ptop_filename="topology.properties"
    
   enable_link_utilization_tracking=true
   link_utilization_tracking_interval_ns=100000000
    
   enable_pingmesh_scheduler=true
   pingmesh_interval_ns=10000000
   pingmesh_endpoint_pairs=all
    
   enable_tcp_flow_scheduler=true
   tcp_flow_schedule_filename="tcp_flow_schedule.csv"
   tcp_flow_enable_logging_for_tcp_flow_ids=set(0,1,2)
    
   enable_udp_burst_scheduler=true
   udp_burst_schedule_filename="udp_burst_schedule.csv"
   udp_burst_enable_logging_for_udp_burst_ids=set(0,1)
   ```
   
   **topology.properties**
   
   ```
   # One ToR with 3 servers
   #
   #    0
   #  / | \
   # 1  2  3
   
   num_nodes=4
   num_undirected_edges=3
   switches=set(0)
   switches_which_are_tors=set(0)
   servers=set(1,2,3)
   undirected_edges=set(0-1,0-2,0-3)
   
   # 10 microseconds delay, 100 Mbit/s, 100 packet queue for all links
   link_channel_delay_ns=map(0-1:10000,0-2:10000,0-3:10000)
   link_device_data_rate_megabit_per_s=map(0->1: 100.0, 1->0: 100.0, 0->2: 100.0, 2->0: 100.0, 0->3: 100.0, 3->0: 100.0)
   link_device_queue=drop_tail(100p)
   link_interface_traffic_control_qdisc=disabled
   ```
   
   **tcp_flow_schedule.csv**
   
   ```
   0,1,3,12500000,0,,
   1,2,3,12500000,500000000,,
   2,2,3,12500000,500000000,,
   ```
   
   **udp_burst_schedule.csv**
   
   ```
   0,1,2,50,0,5000000000,,
   1,2,1,50,0,5000000000,,
   ```
   
2. Your folder structure should as such be:

   ```
   example_run
   |-- config_ns3.properties
   |-- topology.properties
   |-- tcp_flow_schedule.csv
   |-- udp_burst_schedule.csv
   ```

3. Into your ns-3 `scratch/` folder create a file named `my_main.cc`

4. The following is an example code for `scratch/my_main.cc`:

    ```c++
    #include <map>
    #include <iostream>
    #include <fstream>
    #include <string>
    #include <ctime>
    #include <iostream>
    #include <fstream>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <chrono>
    #include <stdexcept>
    #include "ns3/basic-simulation.h"
    #include "ns3/tcp-flow-scheduler.h"
    #include "ns3/udp-burst-scheduler.h"
    #include "ns3/pingmesh-scheduler.h"
    #include "ns3/topology-ptop.h"
    #include "ns3/tcp-optimizer.h"
    #include "ns3/arbiter-ecmp-helper.h"
    #include "ns3/ipv4-arbiter-routing-helper.h"
    #include "ns3/ptop-link-utilization-tracker-helper.h"
    #include "ns3/ptop-queue-utilization-tracker-helper.h"
    
    using namespace ns3;
    
    int main(int argc, char *argv[]) {
    
        // No buffering of printf
        setbuf(stdout, nullptr);
    
        // Retrieve run directory
        CommandLine cmd;
        std::string run_dir = "";
        cmd.Usage("Usage: ./waf --run=\"basic-sim-main-full --run_dir='<path/to/run/directory>'\"");
        cmd.AddValue("run_dir",  "Run directory", run_dir);
        cmd.Parse(argc, argv);
        if (run_dir.compare("") == 0) {
            printf("Usage: ./waf --run=\"basic-sim-main-full --run_dir='<path/to/run/directory>'\"");
            return 0;
        }
    
        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(run_dir);
    
        // Read point-to-point topology, and install routing arbiters
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
    
        // Install link utilization trackers
        PtopLinkUtilizationTrackerHelper linkUtilizationTrackerHelper = PtopLinkUtilizationTrackerHelper(basicSimulation, topology); // Requires enable_link_utilization_tracking=true
    
        // Install link queue trackers
        PtopLinkQueueTrackerHelper linkQueueTrackerHelper = PtopLinkQueueTrackerHelper(basicSimulation, topology); // Requires enable_link_queue_tracking=true
       
        // Optimize TCP
        TcpOptimizer::OptimizeUsingWorstCaseRtt(basicSimulation, topology->GetWorstCaseRttEstimateNs());
    
        // Schedule flows
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology); // Requires enable_tcp_flow_scheduler=true
    
        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology); // Requires enable_udp_burst_scheduler=true
    
        // Schedule pings
        PingmeshScheduler pingmeshScheduler(basicSimulation, topology); // Requires enable_pingmesh_scheduler=true
    
        // Run simulation
        basicSimulation->Run();
    
        // Write flow results
        tcpFlowScheduler.WriteResults();
    
        // Write UDP burst results
        udpBurstScheduler.WriteResults();
    
        // Write pingmesh results
        pingmeshScheduler.WriteResults();
    
        // Write link utilization results
        linkUtilizationTrackerHelper.WriteResults();
    
        // Write link queue results
        linkQueueTrackerHelper.WriteResults();
    
        // Finalize the simulation
        basicSimulation->Finalize();
    
        return 0;
    
    }
    ```

5. Run it by executing in your ns-3 folder:

   ```
   ./waf --run="my_main --run_dir='/your/path/to/example_run'"
   ```
   
   ... or if you also want to save the console output:
   
   ```
   mkdir -p /your/path/to/example_run/logs_ns3
   ./waf --run="my_main --run_dir='/your/path/to/example_run'" 2>&1 | tee /your/path/to/example_run/logs_ns3/console.txt
   ```
   
6. Within `/your/path/to/example_run/logs_ns3` you should find the following output:

   ```
   logs_ns3
   |-- finished.txt
   |-- timing_results.{csv, txt}
   |-- tcp_flows.{csv, txt}
   |-- tcp_flow_{0, 1, 2}_{cwnd, progress, rtt}.csv
   |-- link_utilization.csv
   |-- utilization_compressed.{csv, txt}
   |-- link_utilization_summary.txt
   |-- udp_bursts_{incoming, outgoing}.{csv, txt}
   |-- udp_burst_{0, 1}_{incoming, outgoing}.csv
   |-- pingmesh.{csv, txt}
   ```
   
7. For example, `tcp_flows.txt` will contain:

   ```
   TCP Flow ID     Source    Target    Size            Start time (ns)   End time (ns)     Duration        Sent            Progress     Avg. rate       Finished?     Metadata
   0           1         3         100.00 Mbit     0                 2096381122        2096.38 ms      100.00 Mbit     100.0%       47.7 Mbit/s     YES           
   1           2         3         100.00 Mbit     500000000         4000000000        3500.00 ms      96.08 Mbit      96.1%        27.5 Mbit/s     NO_ONGOING    
   2           2         3         100.00 Mbit     500000000         4000000000        3500.00 ms      71.41 Mbit      71.4%        20.4 Mbit/s     NO_ONGOING             
   ```

8. For example, `udp_bursts_incoming.txt` will contain:

   ```
   UDP burst ID    From      To        Target rate         Start time      Duration        Incoming rate (w/ headers)  Incoming rate (payload)     Packets received   Metadata
   1               2         1         50.00 Mbit/s        0.00 ms         5000.00 ms      49.83 Mbit/s                48.90 Mbit/s                16611              
   0               1         2         50.00 Mbit/s        0.00 ms         5000.00 ms      49.99 Mbit/s                49.06 Mbit/s                16665                         
   ```
   
9. For example, `pingmesh.txt` will contain:

   ```
   Source    Target    Mean latency there    Mean latency back     Min. RTT        Mean RTT        Max. RTT        Smp.std. RTT    Reply arrival
   1         2         4.35 ms               7.76 ms               0.50 ms         12.11 ms        21.70 ms        4.90 ms         399/400 (100%)
   1         3         5.83 ms               0.07 ms               0.05 ms         5.90 ms         16.41 ms        5.97 ms         398/400 (100%)
   2         1         7.73 ms               4.33 ms               0.50 ms         12.05 ms        21.76 ms        4.89 ms         397/400 (99%)
   2         3         9.15 ms               0.06 ms               0.07 ms         9.20 ms         16.74 ms        4.31 ms         398/400 (100%)
   3         1         0.06 ms               5.83 ms               0.05 ms         5.89 ms         16.25 ms        6.00 ms         397/400 (99%)
   3         2         0.07 ms               9.14 ms               0.05 ms         9.20 ms         17.05 ms        4.33 ms         396/400 (99%)                      
   ```

10. For example, `link_utilization_summary.txt` will contain:

   ```
   From     To       Utilization
   0        1        50.56%
   1        0        76.38%
   0        2        51.08%
   2        0        93.77%
   0        3        70.05%
   3        0        1.54%                       
   ```
