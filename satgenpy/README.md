# satgenpy: Satellite network generation

The following five things need to be generated to analyze / simulate a LEO satellite network:

1. `ground_stations.txt` : Describing the ground station properties and locations
2. `tles.txt` : TLEs describing the orbit of all satellites
3. `isls.txt` : Topology of the inter-satellite links
4. `gsl_interfaces_info.txt` : Number of ground-to-satellite link (GSL) interfaces per each node (both satellites and ground stations)
5. `description.txt` : Descriptive information (in particular, max. ISL / GSL length)
6. `dynamic_state/` : Dynamic state which encompasses (a) forwarding state (fstate) and (b) gsl interface bandwidth (gsl_if_bandwidth)

This repository enables you to do so.

## Getting started

1. Python 3.7+

2. The following dependencies need to be installed:

   ```
   pip install numpy astropy ephem networkx sgp4 geopy matplotlib statsmodels
   sudo apt-get install libproj-dev proj-data proj-bin libgeos-dev
   pip install cartopy
   pip install git+https://github.com/snkas/exputilpy.git@v1.6
   ```

## Dynamic state algorithms

There are currently three dynamic state algorithms implemented:

* `algorithm_free_one_only_over_isls` : Only runs for scenarios where there are ISLs.
  It calculates the shortest paths from each ground station / satellite to every ground
  station. It only uses paths which are GS-(SAT)+-GS (in other words, no ground
  station relays). Ground stations and satellites have exactly one interface which
  does not change bandwidth. This interface can send to any other GSL interface ("free").
  
* `algorithm_free_one_only_gs_relays` : Only runs for scenarios where there are no ISLs.
  It calculates the shortest paths from each ground station / satellite to every ground
  station. It only uses paths which are GS-SAT-(GS-SAT)+-GS (in other words, only ground
  station relays). Ground stations and satellites have exactly one interface which
  does not change bandwidth. This interface can send to any other GSL interface ("free").
  
* `algorithm_free_gs_one_sat_many_only_over_isls` : Only runs for scenarios where there are ISLs.
  It calculates the shortest paths from each ground station / satellite to every ground
  station. It only uses paths which are GS-(SAT)+-GS (in other words, no ground
  station relays). Ground stations have exactly one GSL interface which
  does not change bandwidth, and satellites have `# of ground stations` GSL interfaces.
  Importantly, if the satellite sends to a specific ground
  station, it sends from the specific interface on it allocated for that ground station.
  This means that if a satellite is used to reach two ground stations, they will not 
  be bottlenecked by the satellite GSL interface.
  
* `algorithm_paired_many_over_isls` : Only runs for scenarios where there are ISLs.
   It calculates the shortest paths from each ground station / satellite to every ground
   station. It only uses paths which are GS-(SAT)+-GS (in other words, no ground
   station relays). Ground stations have one interface, satellites have 
   `# of ground stations` interfaces. Each ground station interface is bound to the 
   nearest satellite interface (at its index), and only sends there. Bandwidth
   is allocated on both sides based on the number of ground station the satellite connects to.
   (WARNING: THIS IS STILL IN EARLY DEVELOPMENT STAGE)
  

## File formats

### Ground stations

A comma-separated file describing the ground stations.

**Line format for non-extended (without position): ground_stations.basic.txt**

```
[id: int],[name: string],[latitude: float],[longitude: float],[elevation: float]
```

**Example:**

```
0,City: Tokyo; Country: Japan,35.6895,139.69171,0.0
1,City: Delhi; Country: India,28.66667,77.21667,0.0
```

**Line format for extended (with position): ground_stations.txt**

```
[id: int],[name: string],[latitude: float],[longitude: float],[elevation: float],[x cartesian: float],[y cartesian: float],[z cartesian: float]
```

**Example:**

```
0,Tokyo,Japan,37875.951,0.915,35.6895,139.69171,0.0,-3954844.87446858,3354936.24256892,3700264.78797276,0
```


**Notes:**
 * ID needs to be incrementally increased by 1
 * The node ids of the ground stations are assigned after the satellite, so if 
   there are 625 satellites, the first ground station has node ID 625 (but of 
   course ground station id of 0)

### Satellite orbits: tles.txt

This file contains the positions of satellites.

**Format:**

* The first line should contain the number of orbits and the number of satellites per orbit, 
  separated by a space.
* Subsequently, each satellite is described in four lines:

    (1-3) The first three lines define its orbit using the Two-line Element Set Coordinate System:
    
     https://spaceflight.nasa.gov/realdata/sightings/SSapplications/Post/JavaSSOP/SSOP_Help/tle_def.html

**Example:**

```
1 1
Starlink 0
1 00001U 19029BR  18161.59692852  .00001103  00000-0  33518-4 0  9994
2 00001 53.00000   0.7036 0003481 299.7327   0.3331 15.05527065  1773
```

**Notes:**
 - Independent of the name, the satellite ids will be set according to the file's order, starting from 0
 - All satellites should have the same epoch

### Satellite topology: isls.txt

This file contains the inter-satellite links. These links remain in place for the entire simulation.

**Format:**

Each satellite is described in a line consisting of two `int`, the ids of the two satellites that share a link.

```
[from_1] [to_1]
[from_2] [to_2]
...
[from_n] [to_n]
```

**Example:**

```
0 1
0 2
1 2
```

### GSL interfaces info: gsl_interfaces_info.txt

This file contains how many GSLs each node has, and the maximum aggregate bandwidth they can have.

**Format:**

```
[node id],[number of GSL interfaces],[max. aggregate bandwidth]
```

**Example:**

```
329,5,2.0
```

Translates to: node 329 has 5 interfaces which can have an aggregate bandwidth of at most 2.0

### Description: description.txt

This file contains some handy description properties.

**Format:**

```
max_gsl_length_m=<float>
max_isl_length_m=<float>
```

**Example:**

```
max_gsl_length_m=1089686.0000000000
max_isl_length_m=1000000000.0000000000
```

### Forwarding state: dynamic_state/

This directory contains all the dynamic state, which means state which changes over time

#### Forwarding state (fstate)

**Format:**

Each file is named as `fstate_[time in nanoseconds].txt`. Its format is as follows:

```
[current],[dest],[next-hop],[current-interface-id],[next-hop-interface-id]
[current],[dest],[next-hop],[current-interface-id],[next-hop-interface-id]
...
[current],[dest],[next-hop],[current-interface-id],[next-hop-interface-id]
```

**Example:**

```
301,992,340,3,5
```

Translates to: a packet at node 301 destined for 992 will be sent to 340. Node 301 will enqueue it in interface 3 and will send it to the interface 5 of node 340.

**Notes:**

* Only from satellites and ground station node ids as current to the ground stations is encoded in the forwarding state, because satellite are never the destination of a packet during the simulation.

#### GSL interface bandwidth (gsl_if_bandwidth)

**Format:**

Each file is named as `gsl_if_bandwidth_[time in nanoseconds].txt`. Its format is as follows:

```
[node],[interface-id],[bandwidth (unitless)]
[node],[interface-id],[bandwidth (unitless)]
...
[node],[interface-id],[bandwidth (unitless)]
```

**Example:**

```
145,1,0.4
```

Translates to: interface 1 on node 145 has a bandwidth of 0.4
