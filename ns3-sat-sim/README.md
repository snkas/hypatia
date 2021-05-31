# Low earth orbit satellite network simulation using ns-3

Hypatia makes use of ns-3 to simulate the satellite networks at packet-level
granularity. It builds upon two ns-3 modules:

* `satellite` : Satellite movement calculation using SGP-4. This module is used
  by Hypatia to calculate the channel delay for each packet which traverses
  either a GSl or ISL. It was written by Pedro Silva at INESC-TEC. It can
  be found at:
  
  https://gitlab.inesctec.pt/pmms/ns3-satellite
  
  A copy of it is included in this repository with minor modifications.
  It is located at: simulator/src/satellite

* `basic-sim` : Simulation framework to make e.g., running end-to-end 
  TCP flows more easier. It can be found at:
  
  https://github.com/snkas/basic-sim
  
  It is added as git submodule (commit: 3b32597) at: simulator/contrib/basic-sim
  
  The git submodule refers to:
  
  https://github.com/snkas/basic-sim/tree/3b32597c183e1039be7f0bede17d36d354696776


## Getting started

1. Install dependencies (inherited from `basic-sim` ns-3 module):
   ```
   sudo apt-get update
   sudo apt-get -y install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev lcov gnuplot
   pip install numpy statsmodels
   pip install git+https://github.com/snkas/exputilpy.git@v1.6
   git submodule update --init --recursive
   ```

2. Build optimized:
   ```
   bash build.sh --optimized
   ```
