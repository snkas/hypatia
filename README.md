# Hypatia

[![Build Status](https://travis-ci.org/snkas/hypatia.svg?branch=master)](https://travis-ci.org/snkas/hypatia)

Hypatia is a low earth orbit (LEO) satellite network simulation framework.

It consists of four main components:

* `satgenpy` : Python framework to generate LEO satellite networks and generate 
  routing over time over a period of time. It additionally includes several 
  analysis tools to study individual cases. 
  (license: MIT)

* `ns3-sat-sim` : ns-3 based framework which takes as input the state generated 
  by `satgenpy` to perform packet-level simulations over LEO satellite networks.
  It makes use of the `satellite` ns-3 module by Pedro Silva to calculate
  satellite locations over time.
  (license: GNU GPL version 2)
  
* `satviz` : Cesium visualization pipeline to generate interactive satellite network
  visualizations. It makes use of the online Cesium API by generating CesiumJS code.
  The API calls require its user to obtain a Cesium access token (via [https://cesium.com/]()).
  More information can be found in `satviz/README.md`.
  (license: MIT)

* `paper` : Experimental and plotting code to reproduce the experiments and 
  figures which are presented in the paper.
  (license: MIT)
  
(there is a fifth folder called `integration_tests` which is used for integration testing purposes)

This is the code repository introduced and used in "Exploring the “Internet from space” with Hypatia" 
by Simon Kassing*, Debopam Bhattacherjee*, André Baptista Águas, Jens Eirik Saethre and Ankit Singla
(*equal contribution), which is published in the Internet Measurement Conference (IMC) 2020.

BibTeX citation:
```
@inproceedings {hypatia,
    author = {Kassing, Simon and Bhattacherjee, Debopam and Águas, André Baptista and Saethre, Jens Eirik and Singla, Ankit},
    title = {{Exploring the “Internet from space” with Hypatia}},
    booktitle = {{ACM IMC}},
    year = {2020}
}
```

### Visualizations
Most of the visualizations in the paper are available [here](https://leosatsim.github.io/).
All of the visualizations can be regenerated using scripts available in `satviz` as discussed above.

Below are some examples of visualizations:

- SpaceX Starlink 5-shell side-view and top-view. To know the configuration of the shells, click [here](https://leosatsim.github.io/).

<img alt="Starlink side-view" src="https://raw.githubusercontent.com/leosatsim/leosatsim.github.io/master/images/Starlink_side_view.png" width="300px" />
<img alt="Starlink top-view" src="https://raw.githubusercontent.com/leosatsim/leosatsim.github.io/master/images/Starlink_top_view.png" width="300px" />

- Amazon Kuiper 3-shell side-view. To know the configuration of the shells, click [here](https://leosatsim.github.io/kuiper.html).

<img alt="Kuiper side-view" src="https://raw.githubusercontent.com/leosatsim/leosatsim.github.io/master/images/Kuiper_side_view.png" width="45%" />
<img alt="Kuiper top-view" src="https://raw.githubusercontent.com/leosatsim/leosatsim.github.io/master/images/Kuiper_top_view.png" width="45%" />

- RTT changes over time between Paris and Luanda over Starlink 1st shell. Left: 117 ms, Right: 85 ms. Click on the images for 3D interactive visualizations.

<a href="https://leosatsim.github.io/starlink_550_path_Paris_1608_Luanda_1650_46800.html"><img alt="starlink_paris_luanda_long" src="https://raw.githubusercontent.com/leosatsim/leosatsim.github.io/master/images/starlink_paris_luanda_long.png" width="45%" /></a>
<a href="https://leosatsim.github.io/starlink_550_path_Paris_1608_Luanda_1650_139900.html"><img alt="starlink_paris_luanda_short" src="https://raw.githubusercontent.com/leosatsim/leosatsim.github.io/master/images/starlink_paris_luanda_short.png" width="45%" /></a>

- Link utilizations change over time, even with the input traffic being static. For Kuiper 1st shell, path between Chicago and Zhengzhou at 10s (top) and 150s (bottom). Click on the images for 3D interactive visualizations.

<a href="https://leosatsim.github.io/kuiper_630_path_wise_util_Chicago_1193_Zhengzhou_1243_10000.html"><img alt="kuiper_Chicago_Zhengzhou_10s" src="https://raw.githubusercontent.com/leosatsim/leosatsim.github.io/master/images/kuiper_Chicago_Zhengzhou_10s.png" width="90%" /></a>
<a href="https://leosatsim.github.io/kuiper_630_path_wise_util_Chicago_1193_Zhengzhou_1243_150000.html"><img alt="kuiper_Chicago_Zhengzhou_150s" src="https://raw.githubusercontent.com/leosatsim/leosatsim.github.io/master/images/kuiper_Chicago_Zhengzhou_150s.png" width="90%" /></a>

## Getting started

1. System setup:
   - Python version 3.7+
   - Recent Linux operating system (e.g., Ubuntu 18+)

2. Install dependencies:
   ```
   bash hypatia_install_dependencies.sh
   ```
   
3. Build all four modules (as far as possible):
   ```
   bash hypatia_build.sh
   ```
   
4. Run tests:
   ```
   bash hypatia_run_tests.sh
   ```

5. The reproduction of the paper is essentially the tutorial for Hypatia.
   Please navigate to `paper/README.md`.
