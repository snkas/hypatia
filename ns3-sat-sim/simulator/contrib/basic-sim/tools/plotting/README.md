# Plotting

In here you will find a few Python scripts that can be used to easily plot certain parts of a basic simulation.

## Requirements

 * Python 3.7+
 * A recent version of gnuplot: `sudo apt-get install gnuplot`
 * exputil: `pip install git+https://github.com/snkas/exputilpy.git`
 * numpy: `pip install numpy`
 * statsmodels: `pip install statsmodels`

## Getting started

Suppose you have your `logs_ns3` folder at `path/to/run_folder/logs_ns3`

### TCP flow plotting

Plotting TCP flow with id 0 (cwnd, progress, RTT, rate (in 10ms intervals)):

```
cd /path/to/tools/plotting/plot_tcp_flow
python plot_tcp_flow.py path/to/run_folder/logs_ns3 path/to/run_folder/logs_ns3/data path/to/run_folder/logs_ns3/pdf 0 10000000
```

Generating only the rate data file for TCP flow id 0 (in a 10ms interval):

```
cd /path/to/tools/plotting/plot_tcp_flow
python generate_tcp_flow_rate_csv.py path/to/run_folder/logs_ns3 path/to/run_folder/logs_ns3/data 0 10000000
```


### TCP flows ECDF plotting

Plotting all flows' ECDF for flow completion time (FCT) and average throughput:
```
cd /path/to/tools/plotting/plot_tcp_flows_ecdfs
python plot_tcp_flows_ecdfs.py path/to/run_folder/logs_ns3 path/to/run_folder/logs_ns3/data path/to/run_folder/logs_ns3/pdf
```


### Ping plotting

Plotting pings between 21 -> 30 (RTT, # of out-of-order (in 10ms intervals))

```
cd /path/to/tools/plotting/plot_ping
python plot_ping.py path/to/run_folder/logs_ns3 path/to/run_folder/logs_ns3/data path/to/run_folder/logs_ns3/pdf 21 30 10000000
```


### Link utilization plotting

Plotting utilization of link 5 -> 9:
```
cd /path/to/tools/plotting/plot_link_utilization
python plot_link_utilization.py path/to/run_folder/logs_ns3 path/to/run_folder/logs_ns3/data path/to/run_folder/logs_ns3/pdf 5 9
```


### UDP burst plotting

Plotting UDP burst with id 33 (arrived/sent amount, arrived/sent rate (in 100ms intervals), one-way latency):
```
cd /path/to/tools/plotting/plot_udp_burst
python plot_udp_burst.py path/to/run_folder/logs_ns3 path/to/run_folder/logs_ns3/data path/to/run_folder/logs_ns3/pdf 33 100000000
```
