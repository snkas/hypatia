# TCP optimizer

The TCP optimizer can be used to change (hopefully for the better) the default parameters of the TCP stack.

It encompasses the following files:

* `helper/core/tcp-optimizer` - TCP optimizer


## Getting started

1. In your code, import the TCP optimizer:

   ```
   #include "ns3/tcp-optimizer.h"
   ```

3. (Option 1) Before the start of the simulation run, in your code add to optimize just a few generic settings but leaving the timing alone:

    ```c++
    // Optimize TCP by setting some general values
    TcpOptimizer::OptimizeBasic(basicSimulation);
   ```

2. (Option 2) Before the start of the simulation run, in your code add to optimize using the worst-case RTT (does the same generic settings as option 1, in addition to timing settings):

    ```c++
    // Optimize TCP using worst-case RTT assessment
    TcpOptimizer::OptimizeUsingWorstCaseRtt(basicSimulation, topology->GetWorstCaseRttEstimateNs());
    ```
