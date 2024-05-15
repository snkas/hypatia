# TCP flow application

The TCP flow application is the most simple type of application. It schedules flows to start from A to B at time T to transfer X amount of bytes. It saves the results of the flow completion into useful file formats.

It encompasses the following files:

* `model/apps/tcp-flow-send-application.cc/h` - Application which opens a TCP connection and uni-directionally sends data over it
* `model/apps/tcp-flow-sink.cc/h` - Accepts incoming flows and acknowledges incoming data, does not send data back
* `helper/apps/tcp-flow-send-helper.cc/h` - Helper to install flow send applications
* `helper/apps/tcp-flow-sink-helper.cc/h` - Helper to install flow sink applications
* `helper/apps/tcp-flow-scheduler.cc/h` - Reads in a schedule of flows and inserts events for them to start over time. Installs flow sinks on all nodes. Once the run is over, it can write the results to file.
* `helper/apps/tcp-flow-schedule-reader.cc/h` - Schedule reader from file

You can use the application(s) separately, or make use of the flow scheduler (which is recommended).


## Getting started: flow scheduler

1. Add the following to the `config_ns3.properties` in your run folder (for 2 flows that need to be tracked):

   ```
   enable_tcp_flow_scheduler=true
   tcp_flow_schedule_filename="tcp_flow_schedule.csv"
   tcp_flow_enable_logging_for_tcp_flow_ids=set(0,1)
   ```

2. Add the following schedule file `tcp_flow_schedule.csv` to your run folder (three flows from 0 to 1, resp. of size 10/3/34 KB and starting at T=0/10000/30000ns):

   ```
   0,0,1,10000,0,,
   1,0,1,3000,10000,,
   2,0,1,34000,30000,,
   ```

3. In your code, import the pingmesh scheduler:

   ```
   #include "ns3/tcp-flow-scheduler.h"
   ```

3. Before the start of the simulation run, in your code add:

    ```c++
    // Schedule flows
    TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology); // Requires enable_tcp_flow_scheduler=true
    ```
   
4. After the run, in your code add:

    ```c++
    // Write result
    tcpFlowScheduler.WriteResults();
    ```

5. After the run, you should have the flows log files in the `logs_ns3` of your run folder.


## Getting started: directly installing applications

1. In your code, import the TCP flow send and sink helper:

   ```
   #include "ns3/tcp-flow-send-helper.h"
   #include "ns3/tcp-flow-sink-helper.h"
   ```
   
2. Before the start of the simulation run, in your code add:

   ```c++
   // Install a flow sink server on node B: Ptr<Node> node_b
   TcpFlowSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 1024));
   ApplicationContainer app = sink.Install(node_b);
   app.Start(Seconds(0.0));
   
   // Install the client on node A: Ptr<Node> node_a
    TcpFlowSendHelper source(
            "ns3::TcpSocketFactory",
            InetSocketAddress(node_b->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
            1000000000, // Flow size (byte)
            0, // Flow id (must be unique!)
            true, // Enable tracking cwnd / rtt / progress
            m_basicSimulation->GetLogsDir() // Log directory where the tcp_flow_0_{cwnd, rtt, progress}.csv are written
    );
    ApplicationContainer app_flow_0 = source.Install(node_a);
    app.Start(NanoSeconds(0)); // Flow start time (ns)
   ```

3. After the run, in your code add:

   ```c++
   // Retrieve client
   Ptr<TcpFlowSendApplication> tcpFlowSendApp = app_flow_0->GetObject<TcpFlowSendApplication>();

   // Data about this flow
   bool is_completed = tcpFlowSendApp->IsCompleted();
   bool is_conn_failed = tcpFlowSendApp->IsConnFailed();
   bool is_closed_err = tcpFlowSendApp->IsClosedByError();
   bool is_closed_normal = tcpFlowSendApp->IsClosedNormally();
   int64_t sent_byte = tcpFlowSendApp->GetAckedBytes();
   int64_t fct_ns;
   if (is_completed) {
       fct_ns = tcpFlowSendApp->GetCompletionTimeNs() - entry.start_time_ns;
   } else {
       fct_ns = m_simulation_end_time_ns - entry.start_time_ns;
   }
   std::string finished_state;
   if (is_completed) {
       finished_state = "YES";
   } else if (is_conn_failed) {
       finished_state = "NO_CONN_FAIL";
   } else if (is_closed_normal) {
       finished_state = "NO_BAD_CLOSE";
   } else if (is_closed_err) {
       finished_state = "NO_ERR_CLOSE";
   } else {
       finished_state = "NO_ONGOING";
   }
   
   // ... now do whatever you want with this information
   ```


## TCP flow scheduler information

You MUST set the following keys in `config_ns3.properties`:

* `enable_tcp_flow_scheduler` : Must be set to `true`
* `tcp_flow_schedule_filename` : Schedule filename (relative to run folder) (path/to/tcp_flow_schedule.csv)

The following are OPTIONAL in `config_ns3.properties`:

* `tcp_flow_enable_logging_for_tcp_flow_ids` : Set of flow identifiers for which you want logging to file for progress, cwnd and RTT (located at `logs_dir/tcp_flow_[id]_{progress, cwnd, rtt}.csv`). Example value: `set(0, 1`) to log for flows 0 and 1. The file format is: `[tcp_flow_id],[now_in_ns],[progress_byte/cwnd_byte/rtt_ns])`.

**tcp_flow_schedule.csv**

Flow arrival schedule. 

Each line defines a flow as follows:

```
[tcp_flow_id],[from_node_id],[to_node_id],[size_byte],[start_time_ns],[additional_parameters],[metadata]
```

Notes: tcp_flow_id must increment each line. All values except additional_parameters and metadata are mandatory. `additional_parameters` should be set if you want to configure something special for each flow (e.g., different transport protocol, priority). `metadata` you can use for identification later on in the `tcp_flows.csv/txt` logs (e.g., to indicate the workload or coflow it was part of).

**The flow log files**

There are two log files generated by the run in the `logs_ns3` folder within the run folder:

* `tcp_flows.txt` : Flow results in a human readable table.
* `tcp_flows.csv` : Flow results in CSV format for processing with each line:

   ```
   tcp_flow_id,from_node_id,to_node_id,size_byte,start_time_ns,end_time_ns,duration_ns,amount_sent_byte,[finished: YES/CONN_FAIL/NO_BAD_CLOSE/NO_ERR_CLOSE/NO_ONGOING],metadata
   ```

  Finished can have the following outcomes:
  * `YES` = All data was sent and acknowledged fully and there was a normal socket close.
  * `NO_CONN_FAIL` = Connection could not be established (i.e., the handshake failed). Can be caused by sending of SYN / SYN+ACK reaching too many timeouts resulting in no more retries left. In rare cases, it is also possible `NO_BAD_CLOSE` is the outcome of an unsuccessful handshake.
  * `NO_BAD_CLOSE` = Socket was closed prematurely meaning it was closed normally but not all data was transferred.
  * `NO_ERR_CLOSE` = Socket closed because of an error. This can be that a RST was received, or it reached too many timeouts (no more retries left).
  * `NO_ONGOING` = Socket is still sending/receiving and is not yet closed because not all data has been transferred yet.
