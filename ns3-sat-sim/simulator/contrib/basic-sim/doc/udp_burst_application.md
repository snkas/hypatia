# UDP burst application

The UDP burst application is a simple type of application. It schedules bursts, meaning "send from A to B at a rate of X Mbit/s at time T for duration D". It saves the results of the bursts into useful file formats.


It encompasses the following files:

* `model/apps/udp-burst-application.cc/h` - Application which acts as both client and server. One application instance is installed for each node, and the bursts for which it is incoming or outgoing are registered with it.
* `model/apps/udp-burst-info.cc/h` - Information to start a UDP burst.
* `model/apps/id-seq-header.cc/h` - Header put into the UDP burst packets to keep track of which burst it belongs to and which packet in the sequence it is.
* `helper/apps/udp-burst-helper.cc/h` - Helper to install UDP burst applications.
* `helper/apps/udp-burst-scheduler-reader.cc/h` - Reader for the UDP burst schedule.
* `helper/apps/udp-burst-scheduler.cc/h` - Reads in a schedule of bursts. It installs burst applications on all relevant nodes. Once the run is over, it can retrieve the UDP burst application results and write them to file. 

You can use the application(s) separately, or make use of the UDP burst scheduler (which is recommended).


## Getting started: UDP burst scheduler

1. Add the following to the `config_ns3.properties` in your run folder (for 2 UDP bursts that need to be tracked):

   ```
   enable_udp_burst_scheduler=true
   udp_burst_schedule_filename="udp_burst_schedule.csv"
   udp_burst_enable_logging_for_udp_burst_ids=set(0,1)
   ```

2. Add the following schedule file `udp_burst_schedule.csv` to your run folder (two bursts, 50 Mbit/s each, from 0s to 5s, one 1 -> 2 and the other 2 -> 1):

   ```
   0,1,2,50,0,5000000000,,
   1,2,1,50,0,5000000000,,
   ```

3. In your code, import the pingmesh scheduler:

   ```
   #include "ns3/udp-burst-scheduler.h"
   ```

3. Before the start of the simulation run, in your code add:

    ```c++
    // Schedule UDP bursts
    UdpBurstScheduler udpBurstScheduler(basicSimulation, topology); // Requires enable_udp_burst_scheduler=true
    ```
   
4. After the run, in your code add:

    ```c++
    // Write UDP burst results
    udpBurstScheduler.WriteResults();
    ```

5. After the run, you should have the UDP burst log files in the `logs_ns3` of your run folder.


## Getting started: directly installing applications

1. In your code, import the UDP burst helper:

   ```
   #include "ns3/udp-burst-helper.h"
   ```
   
2. Before the start of the simulation run, in your code add:

   ```c++
    // Setup the application on both nodes
    UdpBurstHelper udpBurstHelper(1026, m_basicSimulation->GetLogsDir());
    ApplicationContainer app_a = udpBurstHelper.Install(node_a);
    app_a.Start(Seconds(0.0));
    ApplicationContainer app_b = udpBurstHelper.Install(node_b);
    app_b.Start(Seconds(0.0));
   
   
   // Start a burst from node A (id: 6) to node B (id: 23)
   UdpBurstInfo info = UdpBurstInfo(0, 6, 23, 50, 0, 1000000000, "", "");
   
   // Register the outgoing burst on node A
   app_a.Get(0)->RegisterOutgoingBurst(
            info,
            InetSocketAddress(node_b->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
            true   
   );
   
   // Register the incoming burst on node B
   app_b.Get(0)->RegisterIncomingBurst(info, true);
   ```

3. After the run, in your code add:

   ```c++
   // Each burst you get the number of sent out packets
   std::vector<std::tuple<UdpBurstInfo, uint64_t>> outgoing_bursts = app_a.Get(0)->GetOutgoingBurstsInformation();
   
   // Each burst you get the number of received packets
   std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_bursts = app_b.Get(0)->GetIncomingBurstsInformation();
   ```


## UDP burst scheduler information

You MUST set the following keys in `config_ns3.properties`:

* `enable_udp_burst_scheduler` : Must be set to `true`
* `udp_burst_schedule_filename` : UDP burst schedule filename (relative to run folder) (path/to/udp_burst_schedule.csv)

The following are OPTIONAL in `config_ns3.properties`:

* `udp_burst_enable_logging_for_udp_burst_ids` : Set of UDP burst identifiers for which you want logging of the sent/receive timestamps (located at `logs_dir/udp_burst_[id]_{outgoing, incoming}.csv`). Example value: `set(0, 1`) to log for UDP bursts 0 and 1. 

   The file formats are: `[udp_burst_id],[seq_no],[sent_in_ns]` (outgoing), and `[udp_burst_id],[seq_no],[received_in_ns]` (incoming).

**udp_burst_schedule.csv**

UDP burst start schedule. 

Each line defines a flow as follows:

```
[udp_burst_id],[from_node_id],[to_node_id],[target_rate_megabit_per_s],[start_time_ns],[duration_ns],[additional_parameters],[metadata]
```

Notes: `udp_burst_id` must increment each line. All values except additional_parameters and metadata are mandatory. `additional_parameters` should be set if you want to configure something special for each burst (e.g., different priority). `metadata` you can use for identification later on in the `udp_bursts.csv/txt` logs (e.g., to indicate the workload it was part of).

**The UDP burst log files**

There are four log files generated by the run in the `logs_ns3` folder within the run folder:

* `udp_bursts_{incoming, outgoing}.txt` : UDP burst results in a human readable table.
* `udp_bursts_outgoing.csv` : UDP outgoing burst results in CSV format for processing with each line:

   ```
   [udp_burst_id],[from_node_id],[to_node_id],[target_rate_megabit_per_s],[start_time_ns],[duration_ns],[outgoing_rate_megabit_per_s_with_header],[outgoing_rate_megabit_per_s_only_payload],[packets_sent],[data_sent_byte_incl_headers],[data_sent_byte_only_payload],[metadata]
   ```
  
* `udp_bursts_incoming.csv` : UDP incoming burst results in CSV format for processing with each line:

   ```
   [udp_burst_id],[from_node_id],[to_node_id],[target_rate_megabit_per_s],[start_time_ns],[duration_ns],[incoming_rate_megabit_per_s_with_header],[incoming_rate_megabit_per_s_only_payload],[packets_received],[data_received_byte_incl_headers],[data_received_byte_only_payload],[metadata]
   ```
