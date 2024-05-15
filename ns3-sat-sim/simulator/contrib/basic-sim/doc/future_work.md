# Future work

The backlog of additional work that can be done on the `basic-sim` module:

* Link filters for link (net-device) utilization tracker helper

* Link filters for link (net-device) queue tracker helper

* Incorporate a parameterized selector for RED as queueing discipline
  in the topology configuration (as RED is commonly used)
  
* In the point-to-point topology model, add a mapping option for
  the error model for each link, something along the lines of:
  ```
  link_arrival_error_model=map(0->1: iid_uniform_random(pkt, 0.01), 1->0: iid_uniform_random(pkt, 0.02))
  ```
  
  Links:
  - https://www.nsnam.org/doxygen/point-to-point-net-device_8cc_source.html
  - https://www.nsnam.org/doxygen/simple-error-model_8cc_source.html
  - https://www.nsnam.org/doxygen/error-model_8cc_source.html
  
* Unbuffered version of `log-update-helper.cc/h` which directly writes to file -- this can
  be used for TCP cwnd / progress / RTT tracking.

When upgrading to a new version of ns-3, be sure to check:

* `PointToPointAbHelper` still consistent with the code in `PointToPointHelper`

The goal of `basic-sim` is to be a starting module to do ns-3 experiments in
networks. The following is a list of possible extensions, which should not be
part of `basic-sim`, but can be in a new module depending on `basic-sim`:

* Flowlet routing arbiter
* Long-living TCP application and scheduler
* DCTCP example
